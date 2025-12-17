# Utilisation de packet-forger avec sudo

## Problème

Quand vous utilisez `sudo packet-forger`, la commande n'est pas trouvée car `sudo` ne contient pas `~/.local/bin` dans son PATH.

## Solutions

### Solution 1 : Utiliser le chemin complet

```bash
sudo ~/.local/bin/packet-forger core send --payload /tmp/p3.bin --target 192.168.10.5 --port 80
```

### Solution 2 : Ajouter ~/.local/bin au PATH de sudo

Créez un fichier `/etc/sudoers.d/packet-forger` (nécessite sudo) :

```bash
sudo visudo -f /etc/sudoers.d/packet-forger
```

Ajoutez cette ligne :

```
Defaults secure_path="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/home/kali/.local/bin"
```

**Note** : Remplacez `kali` par votre nom d'utilisateur.

### Solution 3 : Utiliser sudo avec -E pour préserver l'environnement

```bash
sudo -E env "PATH=$PATH" packet-forger core send --payload /tmp/p3.bin --target 192.168.10.5 --port 80
```

### Solution 4 : Créer un alias

Ajoutez à votre `~/.zshrc` ou `~/.bashrc` :

```bash
alias sudo-packet-forger='sudo env "PATH=$PATH" packet-forger'
```

Puis utilisez :

```bash
sudo-packet-forger core send --payload /tmp/p3.bin --target 192.168.10.5 --port 80
```

## Recommandation

La **Solution 1** est la plus simple et la plus sûre. Utilisez le chemin complet avec `sudo`.

