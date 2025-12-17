#!/bin/bash
# Script pour configurer l'auto-complétion dans ~/.zshrc

ZSH_RC="$HOME/.zshrc"
ZSH_COMPLETIONS_DIR="$HOME/.zsh/completions"

echo "[*] Configuration de l'auto-complétion Zsh..."

# Vérifier si le répertoire existe
if [ ! -d "$ZSH_COMPLETIONS_DIR" ]; then
    mkdir -p "$ZSH_COMPLETIONS_DIR"
    echo "[+] Répertoire créé : $ZSH_COMPLETIONS_DIR"
fi

# Vérifier si le fichier _packet-forger existe
if [ ! -f "$ZSH_COMPLETIONS_DIR/_packet-forger" ]; then
    if [ -f "$(dirname "$0")/completions/_packet-forger" ]; then
        cp "$(dirname "$0")/completions/_packet-forger" "$ZSH_COMPLETIONS_DIR/"
        echo "[+] Completion copiée : $ZSH_COMPLETIONS_DIR/_packet-forger"
    else
        echo "[-] Erreur : fichier de completion non trouvé"
        exit 1
    fi
fi

# Ajouter au .zshrc si nécessaire
if [ -f "$ZSH_RC" ]; then
    # Vérifier si fpath est déjà configuré
    if ! grep -q "fpath=(~/.zsh/completions" "$ZSH_RC"; then
        echo "" >> "$ZSH_RC"
        echo "# Auto-complétion packet-forger" >> "$ZSH_RC"
        echo "fpath=(~/.zsh/completions \$fpath)" >> "$ZSH_RC"
        echo "autoload -U compinit && compinit" >> "$ZSH_RC"
        echo "[+] Configuration ajoutée à $ZSH_RC"
    else
        echo "[*] Configuration déjà présente dans $ZSH_RC"
    fi
else
    echo "[*] Création de $ZSH_RC..."
    cat > "$ZSH_RC" << 'EOF'
# Auto-complétion packet-forger
fpath=(~/.zsh/completions $fpath)
autoload -U compinit && compinit
EOF
    echo "[+] Fichier $ZSH_RC créé"
fi

# Vérifier si ~/.local/bin est dans le PATH
if ! grep -q "\$HOME/.local/bin" "$ZSH_RC" 2>/dev/null; then
    echo "" >> "$ZSH_RC"
    echo "# Ajouter ~/.local/bin au PATH" >> "$ZSH_RC"
    echo 'export PATH="$HOME/.local/bin:$PATH"' >> "$ZSH_RC"
    echo "[+] PATH mis à jour dans $ZSH_RC"
fi

echo "[+] Configuration terminée !"
echo ""
echo "[*] Pour activer les changements :"
echo "    source ~/.zshrc"
echo ""
echo "[*] Ou redémarrez votre terminal"

