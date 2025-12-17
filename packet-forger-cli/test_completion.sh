#!/bin/zsh
# Script de test pour l'auto-complétion

echo "[*] Test de l'auto-complétion Zsh pour packet-forger"
echo ""

# Charger la completion
autoload -U compinit
compinit

# Charger la completion packet-forger
fpath=(~/.zsh/completions $fpath)
compdef _packet-forger packet-forger 2>/dev/null

echo "[*] Test 1 : Completion des modules"
echo "packet-forger <TAB> devrait afficher : core payload lab research"
echo ""

echo "[*] Test 2 : Completion des actions core"
echo "packet-forger core <TAB> devrait afficher : send"
echo ""

echo "[*] Test 3 : Completion des options core send"
echo "packet-forger core send <TAB> devrait afficher toutes les options :"
echo "  --payload --target --port --transport-profile --fragment --stealth"
echo "  --source-ip --source-port --ttl --window --interface --help -h --verbose -V"
echo ""

echo "[*] Test 4 : Completion des actions payload"
echo "packet-forger payload <TAB> devrait afficher : build"
echo ""

echo "[*] Test 5 : Completion des plateformes"
echo "packet-forger payload build <TAB> devrait afficher : linux windows"
echo ""

echo "[*] Test 6 : Completion des options payload"
echo "packet-forger payload build linux reverse <TAB> devrait afficher :"
echo "  --lhost --lport --output --polymorphic --asm --encode-xor --help -h --verbose -V"
echo ""

echo "[+] Pour tester manuellement :"
echo "  1. Ouvrez un nouveau terminal"
echo "  2. Tapez : packet-forger core send <TAB><TAB>"
echo "  3. Vous devriez voir toutes les options listées"

