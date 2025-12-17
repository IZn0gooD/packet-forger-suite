#ifndef PAYLOAD_GENERATOR_H
#define PAYLOAD_GENERATOR_H

#include <stdio.h>
#include <stddef.h>

/**
 * @file payload_generator.h
 * @brief Déclarations pour la génération de payloads (shellcode)
 * 
 * Ce module génère des payloads binaires exécutables pour Linux et Windows.
 * Il ne forge PAS de paquets réseau.
 */

// ============================================
// Génération Shellcode Linux
// ============================================

/**
 * @brief Génère un shellcode reverse shell Linux x64
 * @param lhost: IP du listener (attaquant)
 * @param lport: Port du listener
 * @param shellcode: Buffer de sortie
 * @return Taille du shellcode généré, ou -1 si erreur
 */
int generate_shellcode(const char *lhost, int lport, unsigned char *shellcode);

/**
 * @brief Génère un shellcode bind shell Linux x64
 * @param lport: Port d'écoute
 * @param shellcode: Buffer de sortie
 * @return Taille du shellcode généré, ou -1 si erreur
 */
int generate_bind_shell_linux(int lport, unsigned char *shellcode);

// ============================================
// Génération Shellcode Windows
// ============================================

/**
 * @brief Génère un shellcode reverse shell Windows x64 (template msfvenom)
 * @param lhost: IP du listener (attaquant)
 * @param lport: Port du listener
 * @param shellcode: Buffer de sortie
 * @return Taille du shellcode généré, ou -1 si erreur
 */
int generate_shellcode_windows(const char *lhost, int lport, unsigned char *shellcode);

/**
 * @brief Modifie l'IP et le port dans un shellcode Windows existant
 * @param shellcode: Shellcode à modifier
 * @param shellcode_len: Taille du shellcode
 * @param lhost: Nouvelle IP
 * @param lport: Nouveau port
 * @return 0 si succès, -1 si erreur
 */
int modify_windows_shellcode_ip_port(unsigned char *shellcode, int shellcode_len, const char *lhost, int lport);

/**
 * @brief Génère un shellcode Windows x64 complet avec résolution dynamique d'API
 * @param lhost: IP du listener (attaquant)
 * @param lport: Port du listener
 * @param shellcode: Buffer de sortie
 * @param shellcode_size: Taille maximale du buffer
 * @return Taille du shellcode généré, ou -1 si erreur
 */
int generate_windows_shellcode_dynamic(const char *lhost, int lport, unsigned char *shellcode, int shellcode_size);

// ============================================
// Génération en Assembleur (NASM)
// ============================================

/**
 * @brief Génère un reverse shell Linux en assembleur NASM
 * @param lhost: IP du listener
 * @param lport: Port du listener
 * @param output_file: Fichier de sortie (.asm)
 * @return 0 si succès, -1 si erreur
 */
int generate_asm_reverse_shell_linux(const char *lhost, int lport, const char *output_file);

/**
 * @brief Génère un bind shell Linux en assembleur NASM
 * @param lport: Port d'écoute
 * @param output_file: Fichier de sortie (.asm)
 * @return 0 si succès, -1 si erreur
 */
int generate_asm_bind_shell_linux(int lport, const char *output_file);

/**
 * @brief Génère un reverse shell Windows en assembleur NASM (complet)
 * @param lhost: IP du listener
 * @param lport: Port du listener
 * @param output_file: Fichier de sortie (.asm)
 * @return 0 si succès, -1 si erreur
 */
int generate_windows_asm_reverse_shell_complete(const char *lhost, int lport, const char *output_file);

/**
 * @brief Assemble un fichier NASM et extrait le shellcode binaire
 * @param asm_file: Fichier assembleur (.asm)
 * @param shellcode: Buffer de sortie
 * @param shellcode_size: Taille maximale du buffer
 * @return Taille du shellcode extrait, ou -1 si erreur
 */
int assemble_nasm_shellcode(const char *asm_file, unsigned char *shellcode, int shellcode_size);

/**
 * @brief Génère et assemble un shellcode (Linux ou Windows)
 * @param lhost: IP du listener
 * @param lport: Port du listener
 * @param platform: "linux" ou "windows"
 * @param shellcode: Buffer de sortie
 * @param shellcode_size: Taille maximale du buffer
 * @return Taille du shellcode généré, ou -1 si erreur
 */
int generate_and_assemble_shellcode(const char *lhost, int lport, const char *platform, unsigned char *shellcode, int shellcode_size);

// ============================================
// Shellcode Polymorphe
// ============================================

/**
 * @brief Génère un shellcode polymorphe Linux (anti-signature)
 * @param lhost: IP du listener
 * @param lport: Port du listener
 * @param output_file: Fichier de sortie (.asm)
 * @return 0 si succès, -1 si erreur
 */
int generate_polymorphic_asm_reverse_shell_linux(const char *lhost, int lport, const char *output_file);

/**
 * @brief Génère un shellcode polymorphe Windows (anti-signature)
 * @param lhost: IP du listener
 * @param lport: Port du listener
 * @param output_file: Fichier de sortie (.asm)
 * @return 0 si succès, -1 si erreur
 */
int generate_polymorphic_asm_reverse_shell_windows(const char *lhost, int lport, const char *output_file);

/**
 * @brief Génère un shellcode polymorphe (Linux ou Windows)
 * @param lhost: IP du listener
 * @param lport: Port du listener
 * @param platform: "linux" ou "windows"
 * @param shellcode: Buffer de sortie
 * @param shellcode_size: Taille maximale du buffer
 * @return Taille du shellcode généré, ou -1 si erreur
 */
int generate_polymorphic_shellcode(const char *lhost, int lport, const char *platform, unsigned char *shellcode, int shellcode_size);

// ============================================
// Encodage Shellcode
// ============================================

/**
 * @brief Encode un shellcode avec XOR
 * @param shellcode: Shellcode à encoder
 * @param shellcode_len: Taille du shellcode
 * @param encoded: Buffer de sortie
 * @param key_out: Clé XOR utilisée (sortie)
 * @return Taille du shellcode encodé, ou -1 si erreur
 */
int encode_shellcode_xor(const unsigned char *shellcode, int shellcode_len, unsigned char *encoded, unsigned char *key_out);

// ============================================
// Format de Sortie
// ============================================

/**
 * @brief Génère un fichier metadata JSON pour un payload
 * @param output_file: Fichier binaire de sortie (.bin)
 * @param platform: Plateforme ("linux" ou "windows")
 * @param type: Type de payload ("reverse_shell", "bind_shell")
 * @param size: Taille du payload en bytes
 * @param polymorphic: 1 si polymorphe, 0 sinon
 * @param encoded: 1 si encodé, 0 sinon
 * @param lhost: IP du listener (optionnel, NULL si bind shell)
 * @param lport: Port du listener
 * @return 0 si succès, -1 si erreur
 */
int generate_metadata_json(const char *output_file, const char *platform, const char *type,
                          size_t size, int polymorphic, int encoded,
                          const char *lhost, int lport);

/**
 * @brief Sauvegarde un payload dans un fichier binaire
 * @param shellcode: Payload à sauvegarder
 * @param shellcode_len: Taille du payload
 * @param output_file: Fichier de sortie (.bin)
 * @return 0 si succès, -1 si erreur
 */
int save_payload_binary(const unsigned char *shellcode, int shellcode_len, const char *output_file);

#endif // PAYLOAD_GENERATOR_H

