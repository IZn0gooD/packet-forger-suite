/**
 * @file main.c
 * @brief CLI unifié pour Packet Forger Suite
 * 
 * Interface unifiée similaire à Metasploit/Impacket avec sous-commandes :
 *   packet-forger core <action> [options]
 *   packet-forger payload <action> [options]
 *   packet-forger lab <action> [options]
 *   packet-forger research <action> [options]
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <libgen.h>
#include <sys/stat.h>

#define VERSION "3.0.0"
#define MAX_PATH_LEN 1024

// Variables globales pour la verbosité
static int verbose_mode = 0;

// Codes de couleur ANSI
#define COLOR_RESET   "\033[0m"
#define COLOR_BOLD    "\033[1m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"

// Macros pour les messages colorés
#define MSG_INFO(fmt, ...)  printf(COLOR_CYAN "[*] " COLOR_RESET fmt "\n", ##__VA_ARGS__)
#define MSG_SUCCESS(fmt, ...) printf(COLOR_GREEN "[+] " COLOR_RESET fmt "\n", ##__VA_ARGS__)
#define MSG_WARNING(fmt, ...) fprintf(stderr, COLOR_YELLOW "[!] " COLOR_RESET fmt "\n", ##__VA_ARGS__)
#define MSG_ERROR(fmt, ...) fprintf(stderr, COLOR_RED "[-] " COLOR_RESET fmt "\n", ##__VA_ARGS__)
#define MSG_VERBOSE(fmt, ...) if (verbose_mode) printf(COLOR_BLUE "[~] " COLOR_RESET fmt "\n", ##__VA_ARGS__)

/**
 * Exécute une commande système en gérant le problème getcwd()
 * Change vers un répertoire sûr si le répertoire actuel n'existe plus
 */
static int safe_system(const char *cmd) {
    char old_cwd[PATH_MAX];
    int cwd_valid = (getcwd(old_cwd, sizeof(old_cwd)) != NULL);
    
    // Si le répertoire actuel n'existe plus, changer vers HOME ou /tmp
    if (!cwd_valid) {
        char *safe_dir = getenv("HOME");
        if (!safe_dir) safe_dir = "/tmp";
        if (chdir(safe_dir) != 0) {
            MSG_VERBOSE("Impossible de changer vers %s, utilisation de /tmp", safe_dir);
            chdir("/tmp");
        }
    }
    
    int ret = system(cmd);
    
    // Restaurer le répertoire de travail si possible
    if (cwd_valid && old_cwd[0] != '\0') {
        chdir(old_cwd);  // Ignorer l'erreur si on ne peut pas restaurer
    }
    
    return ret;
}

/**
 * Trouve le chemin du répertoire de la suite
 */
static char *find_suite_root(void) {
    static char suite_root[PATH_MAX];
    char exe_path[PATH_MAX];
    char exe_path_copy[PATH_MAX];
    ssize_t len;
    
    // Essayer de lire le lien symbolique /proc/self/exe
    len = readlink("/proc/self/exe", exe_path, sizeof(exe_path) - 1);
    if (len != -1) {
        exe_path[len] = '\0';
        
        // Si le binaire est dans ~/.local/bin, chercher la suite dans VSCODE
        if (strstr(exe_path, ".local/bin") != NULL) {
            char *home = getenv("HOME");
            if (home) {
                snprintf(suite_root, sizeof(suite_root), "%s/VSCODE/packet-forger-suite", home);
                // Vérifier si le répertoire existe
                if (access(suite_root, F_OK) == 0) {
                    return suite_root;
                }
            }
        }
        
        // Sinon, remonter depuis l'exécutable
        strncpy(exe_path_copy, exe_path, sizeof(exe_path_copy) - 1);
        exe_path_copy[sizeof(exe_path_copy) - 1] = '\0';
        
        char *dir1 = dirname(exe_path_copy);  // bin -> packet-forger-cli ou .local/bin -> .local
        char dir1_copy[PATH_MAX];
        strncpy(dir1_copy, dir1, sizeof(dir1_copy) - 1);
        dir1_copy[sizeof(dir1_copy) - 1] = '\0';
        
        char *dir2 = dirname(dir1_copy);      // packet-forger-cli -> packet-forger-cli ou .local -> HOME
        
        // Si on est dans .local/bin, remonter à HOME puis chercher VSCODE
        if (strstr(exe_path, ".local/bin") != NULL) {
            char *home = getenv("HOME");
            if (home) {
                snprintf(suite_root, sizeof(suite_root), "%s/VSCODE/packet-forger-suite", home);
                if (access(suite_root, F_OK) == 0) {
                    return suite_root;
                }
            }
        } else {
            // Sinon, remonter normalement
            snprintf(suite_root, sizeof(suite_root), "%s/..", dir2);
            char resolved[PATH_MAX];
            if (realpath(suite_root, resolved) != NULL) {
                strncpy(suite_root, resolved, sizeof(suite_root) - 1);
                suite_root[sizeof(suite_root) - 1] = '\0';
                return suite_root;
            }
        }
    }
    
    // Fallback : chercher dans VSCODE depuis HOME
    char *home = getenv("HOME");
    if (home) {
        snprintf(suite_root, sizeof(suite_root), "%s/VSCODE/packet-forger-suite", home);
        if (access(suite_root, F_OK) == 0) {
            return suite_root;
        }
    }
    
    // Dernier fallback : utiliser PWD (mais attention, peut être invalide)
    char *pwd = getenv("PWD");
    if (pwd && access(pwd, F_OK) == 0) {
        // Si PWD est dans packet-forger-suite, l'utiliser
        if (strstr(pwd, "packet-forger-suite") != NULL) {
            // Extraire le chemin jusqu'à packet-forger-suite
            char *suite_pos = strstr(pwd, "packet-forger-suite");
            size_t pos = suite_pos - pwd;
            strncpy(suite_root, pwd, pos + strlen("packet-forger-suite"));
            suite_root[pos + strlen("packet-forger-suite")] = '\0';
            return suite_root;
        }
        snprintf(suite_root, sizeof(suite_root), "%s", pwd);
        return suite_root;
    }
    
    // Dernier recours
    return "/home/kali/VSCODE/packet-forger-suite";
}

/**
 * Affiche l'aide principale
 */
void print_help(void) {
    printf(COLOR_BOLD "Packet Forger Suite - CLI Unifié\n" COLOR_RESET);
    printf("Version: %s\n", VERSION);
    printf("\n");
    printf("Usage: packet-forger <module> <action> [options]\n");
    printf("\n");
    printf(COLOR_BOLD "Options globales:\n" COLOR_RESET);
    printf("  --verbose, -V    Mode verbeux (affiche plus de détails)\n");
    printf("  --help, -h       Afficher cette aide\n");
    printf("  --version, -v    Afficher la version\n");
    printf("\n");
    printf(COLOR_BOLD "Modules disponibles:\n" COLOR_RESET);
    printf("  " COLOR_CYAN "core" COLOR_RESET "      Transport de payloads via TCP/IP (forge de paquets)\n");
    printf("  " COLOR_CYAN "payload" COLOR_RESET "   Génération de shellcode native et polymorphe\n");
    printf("  " COLOR_CYAN "lab" COLOR_RESET "       Tests et CVEs pédagogiques\n");
    printf("  " COLOR_CYAN "research" COLOR_RESET "  Documentation théorique (ASLR, DEP, ROP, etc.)\n");
    printf("\n");
    printf(COLOR_BOLD "Exemples:\n" COLOR_RESET);
    printf("  # Générer un payload\n");
    printf("  packet-forger payload build windows reverse --lhost 192.168.10.20 --lport 4444\n");
    printf("\n");
    printf("  # Transporter un payload\n");
    printf("  packet-forger core send --payload payload.bin --target 192.168.10.5 --port 80\n");
    printf("\n");
    printf("  # Workflow complet (génération + transport)\n");
    printf("  packet-forger payload build windows reverse --lhost 192.168.10.20 --lport 4444 --output /tmp/p.bin && \\\n");
    printf("  packet-forger core send --payload /tmp/p.bin --target 192.168.10.5 --port 80\n");
    printf("\n");
    printf("  # Lister les CVEs disponibles\n");
    printf("  packet-forger lab list\n");
    printf("\n");
    printf("  # Consulter la documentation recherche\n");
    printf("  packet-forger research show ASLR\n");
    printf("\n");
    printf("Pour l'aide d'un module spécifique:\n");
    printf("  packet-forger <module> --help\n");
    printf("\n");
}

/**
 * Exécute une commande du module core
 */
int execute_core(int argc, char *argv[]) {
    if (argc < 3 || (strcmp(argv[2], "--help") == 0 || strcmp(argv[2], "-h") == 0)) {
        printf(COLOR_BOLD "Usage: packet-forger core <action> [options]\n" COLOR_RESET);
        printf("\n");
        printf("Actions disponibles:\n");
        printf("  " COLOR_CYAN "send" COLOR_RESET "              Transporter un payload via TCP/IP\n");
        printf("\n");
        printf("Options pour 'send':\n");
        printf("  --payload <file>          Fichier payload binaire\n");
        printf("  --target <ip>             IP cible\n");
        printf("  --port <port>             Port cible\n");
        printf("  --transport-profile <os>  Profil TCP (linux/windows/custom)\n");
        printf("  --fragment                Fragmentation IP\n");
        printf("  --stealth                 Mode stealth (TTL/window variés)\n");
        printf("  --interface <if>          Interface réseau\n");
        printf("\n");
        printf("Exemples:\n");
        printf("  packet-forger core send --payload payload.bin --target 192.168.10.5 --port 80\n");
        printf("  packet-forger core send --payload p.bin --target 192.168.10.5 --port 445 --stealth\n");
        return 0;
    }
    
    const char *action = argv[2];
    
    if (strcmp(action, "send") == 0) {
        MSG_INFO("Module: " COLOR_BOLD "core" COLOR_RESET " - Transport de payloads");
        MSG_VERBOSE("Recherche du binaire packet-forger-core...");
        
        char cmd[MAX_PATH_LEN];
        char *core_path = getenv("PACKET_FORGER_CORE_PATH");
        
        if (!core_path) {
            char *root = find_suite_root();
            snprintf(cmd, sizeof(cmd), "%s/packet-forger-core/bin/packet-forger-core", root);
            MSG_VERBOSE("Chemin détecté automatiquement: %s", cmd);
        } else {
            snprintf(cmd, sizeof(cmd), "%s/packet-forger-core", core_path);
            MSG_VERBOSE("Chemin depuis variable d'environnement: %s", cmd);
        }
        
        // Vérifier si le binaire existe
        if (access(cmd, X_OK) != 0) {
            MSG_ERROR("Binaire non trouvé: %s", cmd);
            MSG_WARNING("Assurez-vous que packet-forger-core est compilé");
            return 1;
        }
        
        MSG_VERBOSE("Binaire trouvé: %s", cmd);
        
        // Construire la commande complète
        char full_cmd[2048];
        snprintf(full_cmd, sizeof(full_cmd), "%s", cmd);
        
        for (int i = 3; i < argc; i++) {
            // Transmettre tous les arguments aux sous-modules
            strncat(full_cmd, " ", sizeof(full_cmd) - strlen(full_cmd) - 1);
            strncat(full_cmd, argv[i], sizeof(full_cmd) - strlen(full_cmd) - 1);
        }
        
        // Ajouter --verbose si le mode verbose est activé
        if (verbose_mode) {
            strncat(full_cmd, " --verbose", sizeof(full_cmd) - strlen(full_cmd) - 1);
        }
        
        MSG_VERBOSE("Commande complète: %s", full_cmd);
        MSG_INFO("Exécution de la commande...");
        
        int ret = safe_system(full_cmd);
        
        if (ret == 0) {
            MSG_SUCCESS("Commande exécutée avec succès");
        } else {
            MSG_ERROR("La commande a échoué (code: %d)", ret);
        }
        
        return ret;
    }
    
    MSG_ERROR("Action inconnue: '%s'", action);
    MSG_INFO("Actions disponibles: send");
    return 1;
}

/**
 * Exécute une commande du module payload
 */
int execute_payload(int argc, char *argv[]) {
    if (argc < 3 || (strcmp(argv[2], "--help") == 0 || strcmp(argv[2], "-h") == 0)) {
        printf(COLOR_BOLD "Usage: packet-forger payload <action> [options]\n" COLOR_RESET);
        printf("\n");
        printf("Actions disponibles:\n");
        printf("  " COLOR_CYAN "build" COLOR_RESET "              Générer un payload\n");
        printf("\n");
        printf("Options pour 'build':\n");
        printf("  <platform> <type>  Plateforme (linux/windows) et type (reverse/bind)\n");
        printf("  --lhost <ip>       IP du listener (reverse shell)\n");
        printf("  --lport <port>     Port du listener\n");
        printf("  --polymorphic      Génération polymorphe\n");
        printf("  --encode-xor       Encodage XOR\n");
        printf("  --output <file>    Fichier de sortie\n");
        printf("\n");
        printf("Exemples:\n");
        printf("  packet-forger payload build windows reverse --lhost 192.168.10.20 --lport 4444\n");
        printf("  packet-forger payload build linux reverse --lhost 192.168.10.20 --lport 4444 --polymorphic\n");
        return 0;
    }
    
    const char *action = argv[2];
    
    if (strcmp(action, "build") == 0) {
        MSG_INFO("Module: " COLOR_BOLD "payload" COLOR_RESET " - Génération de shellcode");
        MSG_VERBOSE("Recherche du binaire payload-builder...");
        
        char cmd[MAX_PATH_LEN];
        char *payload_path = getenv("PACKET_FORGER_PAYLOAD_PATH");
        
        if (!payload_path) {
            char *root = find_suite_root();
            snprintf(cmd, sizeof(cmd), "%s/packet-forger-payloads/bin/payload-builder", root);
            MSG_VERBOSE("Chemin détecté automatiquement: %s", cmd);
        } else {
            snprintf(cmd, sizeof(cmd), "%s/payload-builder", payload_path);
            MSG_VERBOSE("Chemin depuis variable d'environnement: %s", cmd);
        }
        
        // Vérifier si le binaire existe
        if (access(cmd, X_OK) != 0) {
            MSG_ERROR("Binaire non trouvé: %s", cmd);
            MSG_WARNING("Assurez-vous que packet-forger-payloads est compilé");
            return 1;
        }
        
        MSG_VERBOSE("Binaire trouvé: %s", cmd);
        
        // Construire la commande complète
        char full_cmd[2048];
        snprintf(full_cmd, sizeof(full_cmd), "%s build", cmd);
        
        for (int i = 3; i < argc; i++) {
            // Transmettre tous les arguments aux sous-modules
            strncat(full_cmd, " ", sizeof(full_cmd) - strlen(full_cmd) - 1);
            strncat(full_cmd, argv[i], sizeof(full_cmd) - strlen(full_cmd) - 1);
        }
        
        // Ajouter --verbose si le mode verbose est activé
        if (verbose_mode) {
            strncat(full_cmd, " --verbose", sizeof(full_cmd) - strlen(full_cmd) - 1);
        }
        
        MSG_VERBOSE("Commande complète: %s", full_cmd);
        MSG_INFO("Exécution de la commande...");
        
        int ret = safe_system(full_cmd);
        
        if (ret == 0) {
            MSG_SUCCESS("Commande exécutée avec succès");
        } else {
            MSG_ERROR("La commande a échoué (code: %d)", ret);
        }
        
        return ret;
    }
    
    MSG_ERROR("Action inconnue: '%s'", action);
    MSG_INFO("Actions disponibles: build");
    return 1;
}

/**
 * Exécute une commande du module lab
 */
int execute_lab(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: packet-forger lab <action> [options]\n");
        printf("\n");
        printf("Actions disponibles:\n");
        printf("  list              Lister les CVEs disponibles\n");
        printf("  test <CVE>        Tester un CVE spécifique\n");
        printf("  server <type>     Démarrer un serveur vulnérable\n");
        printf("  show <CVE>        Afficher la documentation d'un CVE\n");
        printf("\n");
        printf("Exemples:\n");
        printf("  packet-forger lab list\n");
        printf("  packet-forger lab show CVE-2015-1635\n");
        printf("  packet-forger lab test CVE-2015-1635\n");
        printf("  packet-forger lab server http\n");
        return 1;
    }
    
    const char *action = argv[2];
    
    if (strcmp(action, "list") == 0) {
        MSG_INFO("CVEs disponibles dans packet-forger-labs:");
        printf("  " COLOR_GREEN "•" COLOR_RESET " CVE-2015-1635 (MS15-034) : HTTP.sys Range DoS\n");
        printf("  " COLOR_GREEN "•" COLOR_RESET " CVE-2020-0796 (SMBGhost)  : SMBv3 Compression Buffer Overflow\n");
        printf("\n");
        MSG_INFO("Pour plus d'informations:");
        printf("  packet-forger lab show <CVE>\n");
        return 0;
    }
    
    if (strcmp(action, "show") == 0) {
        if (argc < 4) {
            MSG_ERROR("Spécifiez un CVE (ex: CVE-2015-1635)");
            return 1;
        }
        
        MSG_INFO("Affichage de la documentation pour: %s", argv[3]);
        MSG_VERBOSE("Recherche du fichier de documentation...");
        
        char doc_path[MAX_PATH_LEN];
        char *root = find_suite_root();
        snprintf(doc_path, sizeof(doc_path), 
                 "%s/packet-forger-labs/docs/%s.md",
                 root,
                 argv[3]);
        
        MSG_VERBOSE("Chemin: %s", doc_path);
        
        char cmd[2048];
        snprintf(cmd, sizeof(cmd), "cat %s 2>/dev/null || echo 'Documentation non trouvée: %s'", 
                 doc_path, doc_path);
        return safe_system(cmd);
    }
    
    if (strcmp(action, "test") == 0) {
        if (argc < 4) {
            MSG_ERROR("Spécifiez un CVE (ex: CVE-2015-1635)");
            return 1;
        }
        
        MSG_INFO("Test du CVE: %s", argv[3]);
        MSG_VERBOSE("Recherche du script de test...");
        
        char script_path[MAX_PATH_LEN];
        char *root = find_suite_root();
        // Convertir CVE-2015-1635 en cve_2015_1635 pour le nom de fichier
        char cve_name[MAX_PATH_LEN];
        strncpy(cve_name, argv[3], sizeof(cve_name) - 1);
        cve_name[sizeof(cve_name) - 1] = '\0';
        // Convertir en minuscules et remplacer les tirets par des underscores
        for (char *p = cve_name; *p; p++) {
            if (*p >= 'A' && *p <= 'Z') {
                *p = *p - 'A' + 'a';  // Convertir en minuscule
            } else if (*p == '-') {
                *p = '_';  // Remplacer les tirets par des underscores
            }
        }
        snprintf(script_path, sizeof(script_path),
                 "%s/packet-forger-labs/scripts/test_%s.sh",
                 root,
                 cve_name);
        
        MSG_VERBOSE("Script: %s", script_path);
        
        if (access(script_path, F_OK) != 0) {
            MSG_ERROR("Script de test non trouvé: %s", script_path);
            return 1;
        }
        
        MSG_INFO("Exécution du script de test...");
        char cmd[2048];
        snprintf(cmd, sizeof(cmd), "bash %s", script_path);
        int ret = safe_system(cmd);
        
        if (ret == 0) {
            MSG_SUCCESS("Test terminé avec succès");
        } else {
            MSG_ERROR("Le test a échoué (code: %d)", ret);
        }
        
        return ret;
    }
    
    if (strcmp(action, "server") == 0) {
        if (argc < 4) {
            fprintf(stderr, "[-] Erreur : spécifiez un type de serveur (http, smb)\n");
            return 1;
        }
        
        char server_path[MAX_PATH_LEN];
        const char *server_type = argv[3];
        
        char *root = find_suite_root();
        if (strcmp(server_type, "http") == 0) {
            snprintf(server_path, sizeof(server_path),
                     "%s/packet-forger-labs/servers/vulnerable_http_server.py",
                     root);
        } else if (strcmp(server_type, "smb") == 0) {
            snprintf(server_path, sizeof(server_path),
                     "%s/packet-forger-labs/servers/vulnerable_smb_server.py",
                     root);
        } else {
            fprintf(stderr, "[-] Erreur : type de serveur inconnu (http, smb)\n");
            return 1;
        }
        
        MSG_INFO("Démarrage du serveur vulnérable: %s", server_type);
        MSG_VERBOSE("Chemin: %s", server_path);
        
        if (access(server_path, F_OK) != 0) {
            MSG_ERROR("Script serveur non trouvé: %s", server_path);
            return 1;
        }
        
        char cmd[2048];
        snprintf(cmd, sizeof(cmd), "python3 %s &", server_path);
        MSG_VERBOSE("Commande: %s", cmd);
        int ret = safe_system(cmd);
        
        if (ret == 0) {
            MSG_SUCCESS("Serveur démarré en arrière-plan");
            MSG_VERBOSE("Le serveur tourne en arrière-plan (utilisez 'ps aux | grep python3' pour voir le PID)");
        } else {
            MSG_ERROR("Échec du démarrage du serveur (code: %d)", ret);
        }
        
        return ret;
    }
    
    fprintf(stderr, "[-] Erreur : action inconnue '%s'\n", action);
    return 1;
}

/**
 * Exécute une commande du module research
 */
int execute_research(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: packet-forger research <action> [topic]\n");
        printf("\n");
        printf("Actions disponibles:\n");
        printf("  list              Lister les sujets de recherche disponibles\n");
        printf("  show <topic>      Afficher la documentation d'un sujet\n");
        printf("\n");
        printf("Sujets disponibles:\n");
        printf("  ASLR              Address Space Layout Randomization\n");
        printf("  DEP_NX            Data Execution Prevention / No-Execute\n");
        printf("  STACK_CANARY      Stack Canary Protection\n");
        printf("  ROP_TEMPLATES     Return-Oriented Programming Templates\n");
        printf("  MEMORY_CORRELATION Network ↔ Memory Correlation\n");
        printf("\n");
        printf("Exemples:\n");
        printf("  packet-forger research list\n");
        printf("  packet-forger research show ASLR\n");
        return 1;
    }
    
    const char *action = argv[2];
    
    if (strcmp(action, "list") == 0) {
        MSG_INFO("Sujets de recherche disponibles:");
        printf("  " COLOR_GREEN "•" COLOR_RESET " ASLR              : Address Space Layout Randomization\n");
        printf("  " COLOR_GREEN "•" COLOR_RESET " DEP_NX            : Data Execution Prevention / No-Execute\n");
        printf("  " COLOR_GREEN "•" COLOR_RESET " STACK_CANARY      : Stack Canary Protection\n");
        printf("  " COLOR_GREEN "•" COLOR_RESET " ROP_TEMPLATES     : Return-Oriented Programming Templates\n");
        printf("  " COLOR_GREEN "•" COLOR_RESET " MEMORY_CORRELATION: Network ↔ Memory Correlation\n");
        printf("\n");
        MSG_INFO("Pour consulter un sujet:");
        printf("  packet-forger research show <topic>\n");
        return 0;
    }
    
    if (strcmp(action, "show") == 0) {
        if (argc < 4) {
            MSG_ERROR("Spécifiez un sujet (ex: ASLR)");
            return 1;
        }
        
        const char *topic = argv[3];
        MSG_INFO("Affichage de la documentation pour: %s", topic);
        MSG_VERBOSE("Recherche du fichier de documentation...");
        
        char doc_path[MAX_PATH_LEN];
        
        // Déterminer le chemin selon le sujet
        char *root = find_suite_root();
        if (strcmp(topic, "ASLR") == 0) {
            snprintf(doc_path, sizeof(doc_path),
                     "%s/packet-forger-research/memory/ASLR.md",
                     root);
        } else if (strcmp(topic, "DEP_NX") == 0 || strcmp(topic, "DEP") == 0 || strcmp(topic, "NX") == 0) {
            snprintf(doc_path, sizeof(doc_path),
                     "%s/packet-forger-research/memory/DEP_NX.md",
                     root);
        } else if (strcmp(topic, "STACK_CANARY") == 0 || strcmp(topic, "CANARY") == 0) {
            snprintf(doc_path, sizeof(doc_path),
                     "%s/packet-forger-research/memory/STACK_CANARY.md",
                     root);
        } else if (strcmp(topic, "ROP_TEMPLATES") == 0 || strcmp(topic, "ROP") == 0) {
            snprintf(doc_path, sizeof(doc_path),
                     "%s/packet-forger-research/rop/TEMPLATES.md",
                     root);
        } else if (strcmp(topic, "MEMORY_CORRELATION") == 0) {
            snprintf(doc_path, sizeof(doc_path),
                     "%s/packet-forger-research/network/MEMORY_CORRELATION.md",
                     root);
        } else {
            MSG_ERROR("Sujet inconnu: '%s'", topic);
            MSG_INFO("Utilisez 'packet-forger research list' pour voir les sujets disponibles");
            return 1;
        }
        
        MSG_VERBOSE("Chemin: %s", doc_path);
        
        char cmd[2048];
        snprintf(cmd, sizeof(cmd), "cat %s 2>/dev/null || echo 'Documentation non trouvée: %s'", 
                 doc_path, doc_path);
        return safe_system(cmd);
    }
    
    fprintf(stderr, "[-] Erreur : action inconnue '%s'\n", action);
    return 1;
}

/**
 * Fonction principale
 */
int main(int argc, char *argv[]) {
    // Parser les options globales (--verbose, --help, --version)
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--verbose") == 0 || strcmp(argv[i], "-V") == 0) {
            verbose_mode = 1;
            // Retirer l'option de la liste des arguments
            for (int j = i; j < argc - 1; j++) {
                argv[j] = argv[j + 1];
            }
            argc--;
            i--;
        }
    }
    
    if (argc < 2) {
        print_help();
        return 1;
    }
    
    const char *module = argv[1];
    
    // Aide générale
    if (strcmp(module, "--help") == 0 || strcmp(module, "-h") == 0) {
        print_help();
        return 0;
    }
    
    // Version
    if (strcmp(module, "--version") == 0 || strcmp(module, "-v") == 0) {
        printf(COLOR_BOLD "Packet Forger Suite %s\n" COLOR_RESET, VERSION);
        return 0;
    }
    
    MSG_VERBOSE("Mode verbeux activé");
    MSG_VERBOSE("Nombre d'arguments: %d", argc);
    MSG_VERBOSE("Module sélectionné: %s", module);
    
    // Router vers le module approprié
    if (strcmp(module, "core") == 0) {
        return execute_core(argc, argv);
    } else if (strcmp(module, "payload") == 0) {
        return execute_payload(argc, argv);
    } else if (strcmp(module, "lab") == 0) {
        return execute_lab(argc, argv);
    } else if (strcmp(module, "research") == 0) {
        return execute_research(argc, argv);
    } else {
        MSG_ERROR("Module inconnu: '%s'", module);
        MSG_INFO("Modules disponibles: core, payload, lab, research");
        MSG_INFO("Utilisez 'packet-forger --help' pour l'aide");
        return 1;
    }
}

