#!/bin/bash
# Script d'installation pour packet-forger (sans sudo)

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BIN_DIR="$HOME/.local/bin"
ZSH_COMPLETIONS_DIR="$HOME/.zsh/completions"
BASH_COMPLETIONS_DIR="$HOME/.local/share/bash-completion/completions"

echo "[*] Installation de packet-forger..."

# Créer les répertoires nécessaires
mkdir -p "$BIN_DIR"
mkdir -p "$ZSH_COMPLETIONS_DIR"
mkdir -p "$BASH_COMPLETIONS_DIR"

# Copier le binaire
if [ -f "$SCRIPT_DIR/bin/packet-forger" ]; then
    cp "$SCRIPT_DIR/bin/packet-forger" "$BIN_DIR/"
    chmod +x "$BIN_DIR/packet-forger"
    echo "[+] Binaire installé : $BIN_DIR/packet-forger"
else
    echo "[-] Erreur : binaire non trouvé. Compilez d'abord avec 'make'"
    exit 1
fi

# Installer la completion Bash
if [ -f "$SCRIPT_DIR/completions/packet-forger.bash" ]; then
    cp "$SCRIPT_DIR/completions/packet-forger.bash" "$BASH_COMPLETIONS_DIR/"
    echo "[+] Completion Bash installée : $BASH_COMPLETIONS_DIR/packet-forger.bash"
fi

# Installer la completion Zsh
if [ -f "$SCRIPT_DIR/completions/_packet-forger" ]; then
    cp "$SCRIPT_DIR/completions/_packet-forger" "$ZSH_COMPLETIONS_DIR/"
    echo "[+] Completion Zsh installée : $ZSH_COMPLETIONS_DIR/_packet-forger"
fi

# Vérifier si ~/.local/bin est dans le PATH
if [[ ":$PATH:" != *":$BIN_DIR:"* ]]; then
    echo ""
    echo "[!] ATTENTION : $BIN_DIR n'est pas dans votre PATH"
    echo "[*] Ajoutez cette ligne à votre ~/.zshrc :"
    echo "    export PATH=\"\$HOME/.local/bin:\$PATH\""
    echo ""
fi

# Vérifier la configuration Zsh
if [ -f "$HOME/.zshrc" ]; then
    if ! grep -q "fpath=(~/.zsh/completions" "$HOME/.zshrc"; then
        echo "[!] Pour activer la completion Zsh, ajoutez à votre ~/.zshrc :"
        echo "    fpath=(~/.zsh/completions \$fpath)"
        echo "    autoload -U compinit && compinit"
        echo ""
    fi
fi

echo "[+] Installation terminée !"
echo ""
echo "[*] Pour utiliser packet-forger :"
echo "    1. Ajoutez ~/.local/bin à votre PATH (voir ci-dessus)"
echo "    2. Rechargez votre shell : source ~/.zshrc"
echo "    3. Testez : packet-forger --help"

