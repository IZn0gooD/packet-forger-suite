# DEP/NX (Data Execution Prevention / No-Execute)

## 📋 Vue d'Ensemble

**DEP** (Windows) et **NX** (Linux) empêchent l'exécution de code dans des régions mémoire marquées comme données (stack, heap).

## 🎯 Objectif

Empêcher l'exécution de shellcode injecté dans la stack ou le heap, forçant l'utilisation de techniques plus avancées comme ROP.

## 🔍 Fonctionnement

### Linux (NX Bit)

Le processeur marque les pages mémoire avec un bit NX (No-Execute) :
- **Stack** : Non exécutable par défaut
- **Heap** : Non exécutable par défaut
- **Code** : Exécutable uniquement

### Windows (DEP)

- **Hardware DEP** : Utilise le bit NX du processeur (AMD) ou XD (Intel)
- **Software DEP** : Protection supplémentaire au niveau logiciel
- **Opt-in/Opt-out** : Applications peuvent désactiver DEP

## 🛡️ Bypass Techniques

### 1. Return-Oriented Programming (ROP)

**Principe** : Utiliser des gadgets existants dans le code exécutable

```
Gadget 1: pop rdi; ret
Gadget 2: pop rsi; ret
Gadget 3: mov rax, rdi; call rsi
```

**Avantages** :
- Pas besoin d'exécuter du code injecté
- Utilise uniquement du code légitime
- Contourne DEP/NX complètement

### 2. Jump-Oriented Programming (JOP)

**Principe** : Utiliser des gadgets qui sautent au lieu de retourner

```
Gadget: jmp [rax+0x10]
```

**Avantages** :
- Plus difficile à détecter (pas de ret)
- Moins de gadgets nécessaires

### 3. Call-Oriented Programming (COP)

**Principe** : Utiliser des gadgets qui appellent des fonctions

```
Gadget: call [rax]
```

### 4. mprotect() / VirtualProtect()

**Principe** : Rendre la stack/heap exécutable via des appels système

**Linux** :
```c
mprotect(stack_addr, size, PROT_READ | PROT_WRITE | PROT_EXEC);
```

**Windows** :
```c
VirtualProtect(stack_addr, size, PAGE_EXECUTE_READWRITE, &old_protect);
```

**Technique ROP** :
1. Construire une chaîne ROP
2. Appeler mprotect()/VirtualProtect()
3. Rendre la stack exécutable
4. Exécuter le shellcode

### 5. Return-to-libc (ret2libc)

**Principe** : Appeler directement des fonctions de libc

**Exemple** :
```python
# Retourner vers system("/bin/sh")
rop_chain = [
    pop_rdi_gadget,
    address_of_binsh_string,
    address_of_system,
]
```

### 6. Return-to-PLT (ret2plt)

**Principe** : Utiliser la Procedure Linkage Table (PLT)

**Avantages** :
- Adresses fixes (non randomisées)
- Pas besoin de leak libc
- Fonctionne même avec ASLR

## 📊 Protection Levels

### Linux

- **NX activé** : Stack et heap non exécutables
- **NX désactivé** : Toutes les pages exécutables (dangereux)

### Windows

- **DEP AlwaysOn** : Toujours activé, ne peut pas être désactivé
- **DEP OptIn** : Activé pour les applications qui l'optent
- **DEP OptOut** : Activé sauf pour les applications qui l'optent
- **DEP désactivé** : Désactivé globalement

## 🔧 Détection et Analyse

### Vérifier NX sur Linux

```bash
# Vérifier les flags d'un binaire
readelf -l binary | grep GNU_STACK
# RWE = Read, Write, Execute (NX désactivé)
# RW  = Read, Write (NX activé)

# Vérifier les permissions mémoire
cat /proc/pid/maps
# r-xp = Read, Execute (code)
# rw-p = Read, Write (stack/heap)
```

### Vérifier DEP sur Windows

```powershell
# Vérifier les flags DEP d'un PE
dumpbin /headers executable.exe | findstr "DLL characteristics"
# NX_COMPAT = DEP activé

# Vérifier la politique DEP système
bcdedit /enum {current} | findstr nx
```

### Analyse avec GDB

```gdb
# Vérifier les permissions de la stack
(gdb) info proc mappings
# Stack : rw-p (non exécutable)

# Tester l'exécution sur la stack
(gdb) set $pc = $rsp
(gdb) continue
# Erreur : Cannot access memory (NX activé)
```

## 🎯 Techniques d'Exploitation Modernes

### ROP Chain Construction

1. **Trouver des gadgets** : `ROPgadget`, `ropper`, `objdump`
2. **Construire la chaîne** : Résoudre les dépendances
3. **Gérer les appels système** : Préparer les registres
4. **Gérer les retours** : Chaîner les gadgets

### Exemple ROP Chain (Linux x64)

```python
# Appel système execve("/bin/sh", NULL, NULL)
rop_chain = [
    pop_rdi,        # pop rdi; ret
    binsh_addr,     # "/bin/sh"
    pop_rsi,        # pop rsi; ret
    0,              # argv = NULL
    pop_rdx,        # pop rdx; ret
    0,              # envp = NULL
    pop_rax,        # pop rax; ret
    59,             # syscall execve
    syscall_gadget, # syscall; ret
]
```

## 📚 Références

- [ROP: Return-Oriented Programming](https://hovav.net/ucsd/talks/blackhat08.html)
- [Windows DEP](https://docs.microsoft.com/en-us/windows/win32/memory/data-execution-prevention)
- [NX Bit](https://en.wikipedia.org/wiki/NX_bit)

## 🎓 Points Clés

1. **DEP/NX force ROP** : Impossible d'exécuter du shellcode injecté
2. **ROP est Turing-complete** : Peut faire tout ce qu'un shellcode peut faire
3. **Complexité accrue** : ROP nécessite plus de préparation
4. **Détection possible** : Analyse statique peut détecter des chaînes ROP

---

**Note** : Cette documentation est théorique. Les techniques de bypass doivent être utilisées uniquement dans un contexte légal et éthique.

