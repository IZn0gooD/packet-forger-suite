/**
 * @file verbose_utils.c
 * @brief Utilitaires pour l'affichage verbeux (hexdump, détails)
 */

#include "../include/verbose_utils.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

// Variable globale pour le mode verbose
int verbose_mode = 0;

/**
 * Affiche un hexdump d'un buffer
 */
void hexdump(const unsigned char *data, size_t len, const char *label) {
    if (!verbose_mode || !data || len == 0) return;
    
    printf("\n[~] %s (%zu bytes):\n", label, len);
    printf("[~] ");
    for (size_t i = 0; i < 16; i++) {
        printf("%02zx ", i);
    }
    printf("\n");
    printf("[~] ");
    for (size_t i = 0; i < 16; i++) {
        printf("---");
    }
    printf("\n");
    
    for (size_t i = 0; i < len; i += 16) {
        printf("[~] %04zx: ", i);
        
        // Hex
        for (size_t j = 0; j < 16; j++) {
            if (i + j < len) {
                printf("%02x ", data[i + j]);
            } else {
                printf("   ");
            }
        }
        
        printf(" |");
        
        // ASCII
        for (size_t j = 0; j < 16 && i + j < len; j++) {
            unsigned char c = data[i + j];
            if (c >= 32 && c < 127) {
                printf("%c", c);
            } else {
                printf(".");
            }
        }
        
        printf("|\n");
    }
    printf("\n");
}

/**
 * Affiche les détails d'un payload généré
 */
void print_payload_details(const unsigned char *payload, size_t len, 
                          const char *platform, const char *type,
                          int polymorphic, int encoded, unsigned char xor_key) {
    printf("\n[~] ===== Détails du Payload =====\n");
    printf("[~] Taille totale : %zu bytes\n", len);
    printf("[~] Plateforme : %s\n", platform);
    printf("[~] Type : %s\n", type);
    
    if (polymorphic) {
        printf("[~] Mode polymorphe : Activé\n");
    }
    
    if (encoded) {
        printf("[~] Encodage XOR : Activé (clé: 0x%02x)\n", xor_key);
    }
    
    // Analyse des premiers bytes pour identifier le type
    if (len >= 4) {
        printf("[~] Magic bytes (4 premiers): ");
        for (size_t i = 0; i < 4 && i < len; i++) {
            printf("0x%02x ", payload[i]);
        }
        printf("\n");
    }
    
    // Statistiques
    int null_bytes = 0;
    int printable = 0;
    for (size_t i = 0; i < len; i++) {
        if (payload[i] == 0) null_bytes++;
        if (payload[i] >= 32 && payload[i] < 127) printable++;
    }
    printf("[~] Statistiques :\n");
    printf("[~]   - Bytes null : %d (%.1f%%)\n", null_bytes, (null_bytes * 100.0) / len);
    printf("[~]   - Caractères imprimables : %d (%.1f%%)\n", printable, (printable * 100.0) / len);
    
    // Hexdump complet
    hexdump(payload, len, "Payload complet");
}

