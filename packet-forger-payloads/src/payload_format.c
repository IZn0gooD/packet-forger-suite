#include "../include/payload_generator.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Support JSON optionnel (si json-c disponible)
#ifdef HAVE_JSON_C
#include <json-c/json.h>
#define JSON_SUPPORT 1
#else
#define JSON_SUPPORT 0
#endif

/**
 * @file payload_format.c
 * @brief Format de sortie standard pour les payloads (.bin + .json)
 */

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
                          const char *lhost, int lport) {
    char json_file[512];
    FILE *fp;
    
    // Générer le nom du fichier JSON
    snprintf(json_file, sizeof(json_file), "%s.json", output_file);
    
#if JSON_SUPPORT
    // Créer l'objet JSON
    json_object *root = json_object_new_object();
    json_object *j_platform = json_object_new_string(platform);
    json_object *j_type = json_object_new_string(type);
    json_object *j_size = json_object_new_int64((int64_t)size);
    json_object *j_polymorphic = json_object_new_boolean(polymorphic);
    json_object *j_encoded = json_object_new_boolean(encoded);
    json_object *j_lport = json_object_new_int(lport);
    
    json_object_object_add(root, "platform", j_platform);
    json_object_object_add(root, "type", j_type);
    json_object_object_add(root, "size", j_size);
    json_object_object_add(root, "polymorphic", j_polymorphic);
    json_object_object_add(root, "encoded", j_encoded);
    json_object_object_add(root, "lport", j_lport);
    
    if (lhost) {
        json_object *j_lhost = json_object_new_string(lhost);
        json_object_object_add(root, "lhost", j_lhost);
    }
    
    // Écrire le JSON dans le fichier
    fp = fopen(json_file, "w");
    if (!fp) {
        json_object_put(root);
        return -1;
    }
    
    const char *json_string = json_object_to_json_string_ext(root, JSON_C_TO_STRING_PRETTY);
    fprintf(fp, "%s\n", json_string);
    fclose(fp);
    
    json_object_put(root);
#else
    // Fallback : JSON manuel si json-c non disponible
    fp = fopen(json_file, "w");
    if (!fp) {
        return -1;
    }
    
    fprintf(fp, "{\n");
    fprintf(fp, "  \"platform\": \"%s\",\n", platform);
    fprintf(fp, "  \"type\": \"%s\",\n", type);
    fprintf(fp, "  \"size\": %zu,\n", size);
    fprintf(fp, "  \"polymorphic\": %s,\n", polymorphic ? "true" : "false");
    fprintf(fp, "  \"encoded\": %s,\n", encoded ? "true" : "false");
    fprintf(fp, "  \"lport\": %d", lport);
    if (lhost) {
        fprintf(fp, ",\n  \"lhost\": \"%s\"", lhost);
    }
    fprintf(fp, "\n}\n");
    fclose(fp);
#endif
    
    printf("[+] Metadata générée : %s\n", json_file);
    return 0;
}

/**
 * @brief Sauvegarde un payload dans un fichier binaire
 * @param shellcode: Payload à sauvegarder
 * @param shellcode_len: Taille du payload
 * @param output_file: Fichier de sortie (.bin)
 * @return 0 si succès, -1 si erreur
 */
int save_payload_binary(const unsigned char *shellcode, int shellcode_len, const char *output_file) {
    FILE *fp;
    
    fp = fopen(output_file, "wb");
    if (!fp) {
        fprintf(stderr, "[-] Erreur : impossible de créer '%s'\n", output_file);
        return -1;
    }
    
    size_t written = fwrite(shellcode, 1, shellcode_len, fp);
    fclose(fp);
    
    if (written != (size_t)shellcode_len) {
        fprintf(stderr, "[-] Erreur : écriture incomplète\n");
        return -1;
    }
    
    printf("[+] Payload sauvegardé : %s (%d bytes)\n", output_file, shellcode_len);
    return 0;
}

