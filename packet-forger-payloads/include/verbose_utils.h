/**
 * @file verbose_utils.h
 * @brief Utilitaires pour l'affichage verbeux
 */

#ifndef VERBOSE_UTILS_H
#define VERBOSE_UTILS_H

#include <stddef.h>
#include <stdint.h>

// Variable globale pour le mode verbose
extern int verbose_mode;

void hexdump(const unsigned char *data, size_t len, const char *label);
void print_payload_details(const unsigned char *payload, size_t len,
                          const char *platform, const char *type,
                          int polymorphic, int encoded, unsigned char xor_key);

#endif

