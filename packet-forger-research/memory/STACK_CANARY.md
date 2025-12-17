# Stack Canary (Stack Protector)

## 📋 Vue d'Ensemble

Le **Stack Canary** (ou Stack Protector) est une valeur aléatoire placée sur la stack avant le pointeur de retour pour détecter les buffer overflows.

## 🎯 Objectif

Détecter les tentatives de corruption de la stack avant que le pointeur de retour ne soit utilisé, empêchant ainsi les exploits de buffer overflow.

## 🔍 Fonctionnement

### Placement du Canary

```
[Variables locales]
[Canary]          ← Valeur aléatoire (ex: 0xdeadbeef)
[Saved RBP]       ← Frame pointer sauvegardé
[Return Address]  ← Adresse de retour
```

### Vérification

Avant chaque `ret`, le canary est vérifié :
```c
if (canary != original_canary) {
    __stack_chk_fail();  // Abort avec message d'erreur
}
```

### Types de Canaries

1. **Terminator Canary** : Contient des bytes null (0x00)
   - Détecte les overflows via `strcpy()` (s'arrête à null)
   - Exemple : `0x000aff0d`

2. **Random Canary** : Valeur complètement aléatoire
   - Plus robuste
   - Exemple : `0x7f3a2b1c`

3. **Random XOR Canary** : XOR avec des valeurs aléatoires
   - Plus difficile à deviner
   - Exemple : `0x12345678 ^ random_value`

## 🛡️ Bypass Techniques

### 1. Information Leakage (Canary Leak)

**Principe** : Fuiter la valeur du canary avant l'overflow

**Techniques** :
- Format string vulnerability
- Buffer overflow partiel (lecture)
- Use-After-Free
- Deserialization

**Exemple** :
```c
// Vulnérabilité : printf sans format
char buffer[64];
read(0, buffer, 100);  // Overflow partiel
printf(buffer);        // Format string → leak canary
```

### 2. Brute Force (Byte-by-Byte)

**Principe** : Deviner le canary byte par byte

**Conditions** :
- Application qui se relance (fork, service)
- Pas de rate limiting
- Canary prévisible (peu d'entropie)

**Technique** :
```python
canary = b''
for byte_pos in range(8):  # 64 bits
    for byte_val in range(256):
        payload = b'A' * offset + canary + bytes([byte_val])
        if try_exploit(payload):
            canary += bytes([byte_val])
            break
```

**Probabilité** : 1/256 par byte = 1/2^64 pour un canary complet (64 bits)

### 3. Partial Overwrite

**Principe** : Modifier uniquement les bytes de poids faible

**Limitations** :
- Nécessite un infoleak partiel
- Moins efficace sur 64 bits

### 4. Bypass via Exception Handling

**Principe** : Déclencher une exception avant la vérification du canary

**Techniques** :
- SEH (Structured Exception Handling) sur Windows
- Signal handlers sur Linux
- Longjmp() avant le ret

### 5. Canary Reuse

**Principe** : Réutiliser le canary d'une fonction parente

**Conditions** :
- Fonction imbriquée
- Canary non réinitialisé
- Accès au canary parent

### 6. Bypass via Format String

**Principe** : Modifier directement le canary via format string

```c
// Écrire une valeur spécifique à l'adresse du canary
printf("%12345x%n", &canary_address);
```

## 📊 Entropie Canary

### Linux

- **32 bits** : 24 bits d'entropie (16,777,216 possibilités)
- **64 bits** : 56 bits d'entropie (72,057,594,037,927,936 possibilités)

### Windows

- **32 bits** : ~24 bits d'entropie
- **64 bits** : ~56 bits d'entropie

**Note** : Le canary contient souvent un byte null (terminator), réduisant l'entropie.

## 🔧 Détection et Analyse

### Vérifier Stack Canary

```bash
# Vérifier les flags de compilation
gcc -fstack-protector      # Canary activé
gcc -fstack-protector-all  # Canary pour toutes les fonctions
gcc -fno-stack-protector   # Canary désactivé

# Vérifier dans le binaire
readelf -s binary | grep __stack_chk
# Présence de __stack_chk_fail = canary activé
```

### Analyse avec GDB

```gdb
# Vérifier le canary
(gdb) x/1gx $fs:0x28
# Linux x64 : canary dans segment FS

# Fuiter le canary
(gdb) print canary
$1 = 0x7f3a2b1c00000000

# Vérifier la vérification
(gdb) disassemble function
# Rechercher : call __stack_chk_fail
```

### Détection d'Overflow

```bash
# Message d'erreur typique
*** stack smashing detected ***: ./binary terminated
Aborted (core dumped)
```

## 🎯 Techniques d'Exploitation

### 1. Leak + Overflow

```python
# Étape 1 : Fuiter le canary
canary = leak_canary()

# Étape 2 : Overflow avec canary préservé
payload = b'A' * offset + canary + rop_chain
exploit(payload)
```

### 2. Bypass via Format String

```python
# Modifier le canary pour qu'il corresponde à notre valeur
payload = fmtstr_payload(offset, {canary_addr: known_value})
```

### 3. Bypass via Exception

```python
# Déclencher une exception avant la vérification
payload = b'A' * offset + exception_trigger + rop_chain
```

## 📚 Références

- [Stack Canaries](https://www.sans.org/reading-room/whitepapers/securecode/stack-smashing-protectors-ssp-1429)
- [GCC Stack Protector](https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html)
- [Canary Bypass Techniques](https://www.exploit-db.com/docs/english/28479-bypassing-canaries-and-aslr.pdf)

## 🎓 Points Clés

1. **Canary n'est pas parfait** : Infoleak permet le bypass
2. **Entropie limitée** : Brute force possible dans certains cas
3. **Détection précoce** : Abort avant l'exploitation
4. **Combinaison avec ASLR** : Défense en profondeur

---

**Note** : Cette documentation est théorique. Les techniques de bypass doivent être utilisées uniquement dans un contexte légal et éthique.

