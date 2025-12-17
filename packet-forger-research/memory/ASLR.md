# ASLR (Address Space Layout Randomization)

## 📋 Vue d'Ensemble

**ASLR** (Address Space Layout Randomization) est une technique de sécurité qui randomise les adresses mémoire où les bibliothèques, la pile, le tas et les segments de code sont chargés.

## 🎯 Objectif

Empêcher un attaquant de prédire les adresses mémoire nécessaires pour exploiter une vulnérabilité, rendant les attaques par ROP (Return-Oriented Programming) et autres techniques d'exploitation plus difficiles.

## 🔍 Fonctionnement

### Linux

- **Randomisation des bibliothèques** : `/lib`, `/usr/lib` chargées à des adresses aléatoires
- **Randomisation de la pile** : Adresse de base de la stack aléatoire
- **Randomisation du tas** : Adresse de base du heap aléatoire
- **Randomisation du code** : Position Position Independent Executable (PIE)

### Windows

- **DLL Randomization** : Bibliothèques système chargées à des adresses aléatoires
- **Stack Randomization** : Base de la stack randomisée
- **Heap Randomization** : Base du heap randomisée
- **PE Randomization** : Exécutables avec ASLR activé

## 🛡️ Bypass Techniques

### 1. Information Leakage (Infoleak)

**Principe** : Fuiter une adresse mémoire pour calculer les offsets

```c
// Exemple : Fuite d'adresse libc
printf("libc base: %p\n", printf);
// Calcul : libc_base = leaked_address - offset_of_printf_in_libc
```

**Techniques** :
- Format string vulnerability
- Buffer overflow partiel (lecture au-delà du buffer)
- Use-After-Free (lecture de pointeurs)
- Deserialization (objets avec pointeurs)

### 2. Partial Overwrite

**Principe** : Modifier uniquement les bytes de poids faible

```
Adresse cible : 0x7f1234567890
Adresse leakée : 0x7f1234567000
Différence : 0x890 (seulement 3 bytes à deviner)
```

**Limitations** :
- Nécessite un infoleak
- Probabilité de succès : 1/4096 (12 bits aléatoires)
- Plus efficace sur 32 bits que 64 bits

### 3. Brute Force

**Principe** : Essayer plusieurs fois jusqu'à trouver la bonne adresse

**Conditions** :
- Application qui se relance automatiquement (fork, service)
- ASLR partiel (seulement quelques bits randomisés)
- Pas de rate limiting

**Exemple** :
```python
# Brute force d'une adresse ROP
for attempt in range(1000):
    payload = build_rop_chain(guess_address)
    if exploit(payload):
        break
```

### 4. Non-Randomized Regions

**Principe** : Utiliser des régions non randomisées

**Régions souvent non randomisées** :
- `.data` et `.bss` (variables globales)
- Sections spécifiques du binaire principal
- Bibliothèques sans ASLR (anciennes DLL Windows)

### 5. JIT Spraying

**Principe** : Forcer le générateur de code JIT à créer du code à des adresses prévisibles

**Technique** :
1. Créer beaucoup de code JIT avec des gadgets ROP
2. Les adresses JIT sont souvent prévisibles
3. Utiliser ces gadgets pour construire la chaîne ROP

## 📊 Entropie ASLR

### Linux

- **32 bits** : ~19 bits d'entropie (524,288 possibilités)
- **64 bits** : ~28 bits d'entropie (268,435,456 possibilités)

### Windows

- **32 bits** : ~8 bits d'entropie (256 possibilités) - ASLR faible
- **64 bits** : ~17 bits d'entropie (131,072 possibilités)

## 🔧 Détection et Analyse

### Vérifier ASLR sur Linux

```bash
# Vérifier si ASLR est activé
cat /proc/sys/kernel/randomize_va_space
# 0 = Désactivé
# 1 = Randomisation conservatrice
# 2 = Randomisation complète

# Désactiver temporairement (pour tests)
sudo sysctl -w kernel.randomize_va_space=0
```

### Vérifier ASLR sur Windows

```powershell
# Vérifier les flags ASLR d'un PE
dumpbin /headers executable.exe | findstr "DLL characteristics"
# DYNAMIC_BASE = ASLR activé
```

### Analyse avec GDB

```gdb
# Vérifier les adresses de bibliothèques
(gdb) info sharedlibrary
# Les adresses changent à chaque exécution si ASLR activé

# Fuiter une adresse
(gdb) print &printf
$1 = (int (*)(const char *, ...)) 0x7ffff7a62800 <printf>
```

## 📚 Références

- [PaX ASLR](https://pax.grsecurity.net/docs/aslr.txt)
- [Windows ASLR](https://docs.microsoft.com/en-us/windows/win32/memory/address-space-layout-randomization)
- [ASLR Bypass Techniques](https://www.exploit-db.com/docs/english/17914-address-space-layout-randomization-(aslr).pdf)

## 🎓 Points Clés

1. **ASLR n'est pas parfait** : Entropie limitée, régions non randomisées
2. **Infoleak est crucial** : La plupart des bypass nécessitent une fuite d'adresse
3. **Combinaison de techniques** : ASLR + DEP + Canary = défense en profondeur
4. **64 bits plus robuste** : Plus d'entropie = plus difficile à bypasser

---

**Note** : Cette documentation est théorique et pédagogique. Les techniques de bypass doivent être utilisées uniquement dans un contexte légal et éthique.

