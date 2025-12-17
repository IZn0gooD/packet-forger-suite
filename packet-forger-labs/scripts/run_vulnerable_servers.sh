#!/bin/bash
# Script pour démarrer tous les serveurs vulnérables

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SERVERS_DIR="$SCRIPT_DIR/../servers"

echo "[*] Démarrage des serveurs vulnérables"
echo "[*] ===================================="
echo ""

# Vérifier Python
if ! command -v python3 &> /dev/null; then
    echo "[-] Python3 non trouvé"
    exit 1
fi

# Démarrer le serveur HTTP vulnérable
echo "[*] Démarrage du serveur HTTP vulnérable (port 8080)..."
cd "$SERVERS_DIR"
python3 vulnerable_http_server.py 8080 &
HTTP_PID=$!
echo "[+] Serveur HTTP démarré (PID: $HTTP_PID)"
echo ""

# Démarrer le serveur SMB vulnérable (nécessite root pour port 445)
if [ "$EUID" -eq 0 ]; then
    echo "[*] Démarrage du serveur SMB vulnérable (port 445)..."
    python3 vulnerable_smb_server.py 445 &
    SMB_PID=$!
    echo "[+] Serveur SMB démarré (PID: $SMB_PID)"
    echo ""
else
    echo "[!] Serveur SMB non démarré (nécessite root pour port 445)"
    echo "[*] Pour démarrer manuellement: sudo python3 vulnerable_smb_server.py 445"
    echo ""
fi

echo "[*] Serveurs démarrés"
echo "[*] Appuyez sur Ctrl+C pour arrêter tous les serveurs"
echo ""

# Attendre Ctrl+C
trap "kill $HTTP_PID $SMB_PID 2>/dev/null; exit" INT TERM
wait

