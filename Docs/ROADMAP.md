# Feuille de route d'Asset Engine

Période : du **lundi 9 novembre** au **vendredi 20 novembre 2026**, jour du rendu, de la soutenance et du code review.

Chacun développe le projet complet sur sa branche, puis les deux versions sont comparées à chaque point commun : la meilleure de chaque module est fusionnée dans `dev`.

Chaque step se termine par **un test qui passe, puis un commit sur sa branche**. Une case se coche dans le commit qui termine le step. Les dates sont indicatives.

## Semaine 1 : le cœur du projet

### Lundi 9 : streams et début des archives

- [x] **Step 1 :** `Core/IO/IInputStream` et `Core/IO/IOutputStream`, les interfaces de lecture et d'écriture
- [x] **Step 2 :** `Core/IO/MemoryStream`, lire et écrire en mémoire
  - [x] Test : écrire puis relire donne les mêmes octets
  - [x] Test : lire au-delà de la fin renvoie moins d'octets, sans planter
  - [x] Test : `Size` est juste, `Seek` hors limites est refusé
  - [x] Test : construire à partir d'octets existants
- [x] **Step 3 :** `Core/IO/FileInputStream` et `Core/IO/FileOutputStream`, lire et écrire un fichier
  - [x] Test : écrire puis relire un fichier, `Seek` après la fin du fichier
  - [x] Test : un fichier absent est signalé
  - [x] Test : réécrire un fichier remplace son contenu
- [x] **Step 4 :** `Serialization/Archive` et `Serialization/Serializers/`, l'archive abstraite et ses règles
  - [x] Test : parcours d'une structure avec des tableaux imbriqués
  - [x] Test : remplissage d'un `vector` et d'une `map` en chargement
  - [x] Test : les concepts acceptent et refusent les bons types

### Mardi 10 : archive binaire

- [x] **Step 5 :** `BinaryArchive`, types primaires et chaînes
  - [x] Test : aller-retour, valeurs limites, chaîne vide
- [ ] **Step 6 :** `BinaryArchive`, tableaux, tableaux imbriqués, structures, maps
  - [ ] Test : aller-retour complet sur un objet qui contient tous ces types
- [ ] **Step 7 :** données invalides ou tronquées
  - [ ] Test : `HasFailed()` sans plantage

### Mercredi 11 : JSON

- [x] **Step 8 :** lib JSON maison, l'écriture
  - [x] Test : chaque type de valeur, imbrication, caractères échappés
- [x] **Step 9 :** lib JSON maison, la lecture
  - [x] Test : parser puis réécrire, JSON invalide refusé
- [x] **Step 10 :** `JsonWriter` et `JsonReader`, les archives JSON
  - [x] Test : le même aller-retour que le binaire, sortie en JSON valide
- [ ] **🤝 Comparaison 1 :** streams et archives, puis fusion de la meilleure version dans `dev`

### Jeudi 12 : réflexion

- [ ] **Step 11 :** `Core/Guid`, l'identifiant de type calculé à la compilation
  - [ ] Test : même nom, même id, vérifié avec `static_assert`
- [ ] **Step 12 :** `Core/RTTI`, la description d'un type : nom, version, bases, héritage multiple
  - [ ] Test : conversion vers la 2ᵉ base, `IsA`, héritage en diamant
- [ ] **Step 13 :** enregistrement avant `main` et gel du registre
  - [ ] Test : le type existe dès `main`, un enregistrement tardif est refusé
- [ ] **Step 14 :** `Core/Factory`
  - [ ] Test : créer un objet depuis son id, id inconnu refusé
- [ ] **Step 15 :** types polymorphes dans les archives
  - [ ] Test : `unique_ptr<Base>` recharge le bon type, pointeur nul

### Vendredi 13 : sécurité

- [ ] **Step 16 :** `Security/Checksum`, le CRC32
  - [ ] Test : `"123456789"` donne `0xCBF43926`
- [ ] **Step 17 :** SHA-256, avec une bibliothèque (consigne du prof, choix à faire)
  - [ ] Test : vecteurs officiels
- [ ] **Step 18 :** le hash à clé (HMAC)
  - [ ] Test : vecteurs officiels, mauvaise clé refusée
- [ ] **Step 19 :** politique de sécurité selon le contexte, vérification à la lecture
  - [ ] Test : octet modifié refusé, CRC32 recalculé refusé en contexte HMAC
- [ ] **🤝 Comparaison 2 :** réflexion et sécurité

### Samedi 14 : assets

- [ ] **Step 20 :** `Asset/Asset` et le format `.asset` (header, métadonnées, payload, digest)
  - [ ] Test : lire les métadonnées sans le payload
- [ ] **Step 21 :** `Asset/AssetRegistry`
  - [ ] Test : `Scan()`, recherches, `registry.bin` reconstruit
- [ ] **Step 22 :** `Asset/AssetManager` en synchrone, avec cache et handle
  - [ ] Test : deux `Load` donnent la même instance, libération automatique

### Dimanche 15 : asynchrone et interface

- [ ] **Step 23 :** `Asset/JobSystem`
  - [ ] Test : 1 000 tâches exécutées une seule fois, arrêt propre
- [ ] **Step 24 :** `LoadAsync` et `Update`
  - [ ] Test : pas de double chargement, callbacks sur le thread principal
- [ ] **Step 25 :** interface ImGui simple, dans `editor/`
  - [ ] Vérification à l'œil
- [ ] **🤝 Comparaison 3 :** assets et chargement

## Semaine 2 : finitions et bonus

- [ ] **Step 26 :** démo complète dans le Sandbox, avec des types d'assets d'exemple
- [ ] **Step 27 :** finaliser l'interface ImGui, qui vaut 5 points
- [ ] **Step 28 :** bonus au choix : mesures de performance, dépendances entre assets, versioning avancé
- [ ] **Step 29 :** contrôle final : Debug et Release, clone neuf, nettoyage des fichiers temporaires
- [ ] **Step 30 :** documents à jour, diagramme, fusion dans `main` et migration vers le GitHub de l'école
- [ ] **Step 31 :** slides et répétition en 15 minutes (3 + 5 + 5 + 2)

## Le minimum à sécuriser

En cas de retard, ces éléments passent en priorité, car le cahier des charges les demande explicitement :

- les archives binaire et texte, avec tous leurs types ;
- le checksum et le hash à clé ;
- le registry ;
- le chargement synchrone et asynchrone ;
- une interface ImGui simple.

Les dépendances entre assets, le versioning avancé et les finitions de l'éditeur peuvent sauter en premier.
