#!/bin/bash
# Script de test pour CVE-2020-0796 (SMBGhost)

TARGET_IP="${1:-127.0.0.1}"
TARGET_PORT="${2:-445}"

echo "[*] Test CVE-2020-0796 (SMBGhost)"
echo "[*] =============================="
echo "[*] Cible: $TARGET_IP:$TARGET_PORT"
echo ""

# Vérifier si smbclient est disponible
if ! command -v smbclient &> /dev/null; then
    echo "[-] smbclient non trouvé. Installation: apt-get install smbclient"
    exit 1
fi

echo "[*] Tentative de connexion SMB..."
echo "[*] Note: Ce script teste uniquement la connectivité"
echo "[*] Pour un test complet, utilisez packet-forger-core avec un payload"
echo ""

# Test de connexion basique
smbclient -L "//$TARGET_IP" -N -p "$TARGET_PORT" 2>&1 | head -20

echo ""
echo "[+] Test terminé"

