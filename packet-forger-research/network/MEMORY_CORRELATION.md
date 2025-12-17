# Corrélation Réseau ↔ Mémoire

## 📋 Vue d'Ensemble

Ce document explore la corrélation entre les données reçues via le réseau et leur représentation en mémoire, essentielle pour comprendre comment les vulnérabilités réseau peuvent mener à des exploits mémoire.

## 🔗 Flux de Données : Réseau → Mémoire

### 1. Réception Réseau

```
[Paquet TCP/IP]
    ↓
[Buffer kernel]
    ↓
[read() / recv()]
    ↓
[Buffer utilisateur]
    ↓
[Parsing / Traitement]
    ↓
[Variables mémoire]
```

### 2. Points de Vulnérabilité

#### Buffer Overflow

```c
char buffer[64];
read(socket_fd, buffer, 1024);  // Overflow si > 64 bytes
```

**Corrélation** :
- **Réseau** : 1024 bytes reçus
- **Mémoire** : 64 bytes alloués
- **Résultat** : Overflow de 960 bytes

#### Format String

```c
char buffer[256];
read(socket_fd, buffer, 256);
printf(buffer);  // Format string si contient %x, %s, etc.
```

**Corrélation** :
- **Réseau** : Chaîne avec format specifiers
- **Mémoire** : Stack lue/écrite via format string
- **Résultat** : Leak ou écriture mémoire

#### Integer Overflow

```c
int size;
read(socket_fd, &size, sizeof(int));
char *buffer = malloc(size);  // Overflow si size négatif
```

**Corrélation** :
- **Réseau** : Entier malformé
- **Mémoire** : Allocation incorrecte
- **Résultat** : Buffer overflow ou crash

## 🎯 Scénarios d'Exploitation

### Scénario 1 : Buffer Overflow via HTTP

```
[Requête HTTP]
POST /upload HTTP/1.1
Content-Length: 10000
[10000 bytes de données]
    ↓
[Code serveur]
char buffer[512];
read(client_fd, buffer, content_length);  // Overflow
    ↓
[Stack corrompue]
[Canary écrasé]
[Return address écrasée]
    ↓
[Exploitation ROP]
```

### Scénario 2 : Format String via SMB

```
[Paquet SMB]
[Champ filename avec %x%x%x%x]
    ↓
[Code serveur]
char filename[256];
strcpy(filename, smb_filename);
printf("Opening: %s\n", filename);  // Format string
    ↓
[Stack leakée]
[Canary leaké]
[ASLR bypassé]
    ↓
[Exploitation ciblée]
```

### Scénario 3 : Deserialization via Protocole Custom

```
[Paquet binaire]
[Structure sérialisée avec pointeurs]
    ↓
[Code serveur]
Object obj = deserialize(data);  // Pointeurs restaurés
obj->method();  // Use-After-Free ou type confusion
    ↓
[Corruption mémoire]
[ROP chain injectée]
    ↓
[Exploitation]
```

## 📊 Mapping Réseau → Mémoire

### Structure de Paquet

```
[En-tête réseau]
    ↓
[En-tête application]
    ↓
[Données payload]
    ↓
[Padding / Checksum]
```

### Structure Mémoire

```
[Variables locales]
    ↓
[Canary]
    ↓
[Saved RBP]
    ↓
[Return Address]
    ↓
[Arguments fonction]
```

### Correspondance

```
Offset réseau 0x00 → Variable locale buffer[0]
Offset réseau 0x40 → Canary (si overflow)
Offset réseau 0x48 → Saved RBP
Offset réseau 0x50 → Return Address (EIP/RIP)
```

## 🔍 Analyse des Protocoles

### HTTP

**Points d'entrée** :
- Headers (User-Agent, Cookie, etc.)
- Body (POST data)
- Query parameters
- Path (URL)

**Vulnérabilités communes** :
- Buffer overflow dans parsing
- Command injection dans paramètres
- SQL injection dans query

### SMB

**Points d'entrée** :
- Filenames
- Share names
- Transaction data
- Compression data

**Vulnérabilités communes** :
- Buffer overflow dans décompression
- Path traversal
- Integer overflow dans calculs de taille

### FTP

**Points d'entrée** :
- Commands
- Filenames
- Directory paths

**Vulnérabilités communes** :
- Buffer overflow dans command parsing
- Format string dans logging

## 🛠️ Techniques d'Analyse

### 1. Capture Réseau

```bash
# Capturer le trafic
tcpdump -i eth0 -w capture.pcap

# Analyser avec Wireshark
wireshark capture.pcap
```

### 2. Analyse Mémoire

```gdb
# Examiner la stack
(gdb) x/20gx $rsp

# Examiner le heap
(gdb) x/20gx $heap_base

# Examiner les registres
(gdb) info registers
```

### 3. Corrélation

```python
# Mapper les données réseau vers la mémoire
network_data = b"A" * 64 + p64(canary) + p64(rop_gadget)
memory_layout = {
    'buffer': network_data[0:64],
    'canary': network_data[64:72],
    'saved_rbp': network_data[72:80],
    'return_addr': network_data[80:88],
}
```

## 📚 Études de Cas

### Cas 1 : CVE-2020-0796 (SMBGhost)

**Réseau** :
- Paquet SMBv3 avec compression
- `OriginalCompressedSegmentSize` très grand

**Mémoire** :
- Buffer alloué basé sur `OriginalCompressedSegmentSize`
- Décompression dépasse le buffer
- Buffer overflow dans le kernel

**Corrélation** :
```
Réseau: OriginalCompressedSegmentSize = 0xFFFFFFFF
    ↓
Mémoire: Buffer alloué = 0xFFFFFFFF bytes (impossible)
    ↓
Résultat: Buffer overflow lors de la décompression
```

### Cas 2 : CVE-2015-1635 (HTTP.sys)

**Réseau** :
- Header HTTP `Range: bytes=0-18446744073709551615`

**Mémoire** :
- Parsing du Range header
- Calcul de taille avec uint64_t
- Overflow dans le calcul

**Corrélation** :
```
Réseau: Range header malformé
    ↓
Mémoire: Calcul de taille overflow
    ↓
Résultat: Crash du pilote kernel (DoS)
```

## 🎓 Points Clés

1. **Tout vient du réseau** : Les données d'exploitation transitent par le réseau
2. **Parsing = Vulnérabilité** : Le parsing réseau est souvent vulnérable
3. **Mémoire = Cible** : L'objectif est de corrompre la mémoire
4. **Protocole = Vecteur** : Chaque protocole a ses points d'entrée

## 📚 Références

- [Network Protocol Analysis](https://www.wireshark.org/docs/)
- [Memory Corruption Attacks](https://owasp.org/www-community/vulnerabilities/Memory_corruption)
- [Protocol Fuzzing](https://owasp.org/www-community/Fuzzing)

---

**Note** : Cette documentation est théorique. Les techniques doivent être utilisées uniquement dans un contexte légal et éthique.

