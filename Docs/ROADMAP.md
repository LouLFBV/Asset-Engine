# Feuille de route d'Asset Engine

Période : du **lundi 9 novembre** au **vendredi 20 novembre 2026**, jour du rendu, de la soutenance et du code review.

Une étape n'est terminée que lorsque ses tests passent. A et B correspondent à la répartition de [ARCHITECTURE.md](ARCHITECTURE.md). Les dates sont indicatives.

## Phase 0 : Base commune (lun. 9, à deux)

- [ ] **Décisions :** répartition, namespace, gestion des erreurs, format `.asset`
- [x] **`IInputStream` et `IOutputStream`**

## Phase 1 : Fondations (10-12 nov.)

### A : Réflexion

- [ ] **`TypeId` à la compilation**
  - [ ] Test : même nom, même id ; noms différents, ids différents
  - [ ] Test : `static_assert` qui prouve le calcul à la compilation
- [ ] **`TypeInfo` et `REFLECT`** (nom, version, champs)
  - [ ] Test : le nom, la version et les champs sont corrects sans créer d'instance
- [ ] **Héritage multiple**
  - [ ] Test : la conversion vers la 2ᵉ base donne le bon pointeur
  - [ ] Test : `IsA` fonctionne à travers plusieurs niveaux
  - [ ] Test : une base virtuelle en diamant n'est comptée qu'une fois
- [ ] **`TypeRegistry` et `Factory`**
  - [ ] Test : un type est déjà enregistré dès l'entrée dans `main`
  - [ ] Test : enregistrer un type après le gel provoque une erreur
  - [ ] Test : un `TypeId` inconnu renvoie une erreur

### B : JSON et intégrité

- [ ] **`JsonWriter`, `JsonValue` et `JsonParser`**
  - [ ] Test : chaque type de valeur, y compris les objets et tableaux imbriqués
  - [ ] Test : parser puis réécrire redonne les mêmes données
  - [ ] Test : caractères échappés et Unicode
  - [ ] Test : un JSON invalide renvoie une erreur, sans planter
- [ ] **`IIntegrityProvider` et `Crc32`**
  - [ ] Test : `"123456789"` donne `0xCBF43926`
  - [ ] Test : un octet modifié change le résultat
- [ ] **`Sha256`**
  - [ ] Test : vecteurs officiels FIPS 180-4
- [ ] **`HmacSha256` et `KeyProvider`**
  - [ ] Test : vecteurs officiels RFC 4231
  - [ ] Test : une mauvaise clé fait échouer la vérification

## Phase 2 : Archives (12-14 nov.)

### A

- [ ] **`IArchive`**, à pousser sur `dev` dès qu'elle est stable
- [ ] **`MemoryStream`**, à pousser sur `dev` avec `IArchive`
  - [ ] Test : écrire puis relire donne les mêmes octets
  - [ ] Test : lire au-delà de la fin renvoie moins d'octets, sans planter
  - [ ] Test : `Seek` et `Size` donnent les bonnes valeurs
- [ ] **Concepts et `Serializer<T>`**
  - [ ] Test : un type non sérialisable ne compile pas (`static_assert` sur le concept)
- [ ] **`BinaryArchive`**
  - [ ] Test d'aller-retour : types primaires et valeurs limites, chaînes vides, tableaux vides
  - [ ] Test d'aller-retour : tableaux imbriqués, structures, `map` et `unordered_map`
  - [ ] Test : `IsLoading` et `IsSaving` donnent la bonne valeur, à l'exécution et à la compilation
  - [ ] Test : varint sur `0`, `127`, `128` et la valeur maximale
  - [ ] Test : un fichier tronqué renvoie une erreur
- [ ] **Types polymorphes**
  - [ ] Test : `unique_ptr<Base>` recharge le bon type dérivé
  - [ ] Test : pointeur nul
  - [ ] Test : héritage multiple, avec les champs de chaque base
- [ ] **Versioning**
  - [ ] Test : un ancien fichier se charge, et les nouveaux champs prennent leur valeur par défaut
- [ ] **`FileStream`**
  - [ ] Test : écrire puis relire sur le disque ; un fichier absent renvoie une erreur

### B

- [ ] **`TextArchive`**
  - [ ] Test : le même test d'aller-retour que pour le binaire
  - [ ] Test : la sortie est du JSON valide
  - [ ] Test : maps à clé non textuelle, champs de même nom dans deux bases
- [ ] **`SecurityPolicy` et `SecureStream`**
  - [ ] Test : un fichier intact se charge
  - [ ] Test : un octet modifié dans le header, le payload ou le digest est refusé
  - [ ] Test : un fichier avec un CRC32 recalculé est refusé en contexte HMAC
  - [ ] Test : aucun octet n'est désérialisé avant la vérification

## Phase 3 : Assets (15-18 nov.)

### A

- [ ] **Format `.asset` et `AssetMetadata`**
  - [ ] Test : écrire puis relire en binaire et en texte
  - [ ] Test : magic incorrect ou version inconnue renvoie une erreur
  - [ ] Test : lire les métadonnées ne lit pas le payload
- [ ] **`AssetRegistry`**
  - [ ] Test : `Scan()` trouve tous les assets d'un dossier
  - [ ] Test : recherche par id, type, tag et nom
  - [ ] Test : collision d'`AssetId` signalée
- [ ] **`registry.bin`**
  - [ ] Test : un registry supprimé se reconstruit à l'identique
  - [ ] Test : un registry corrompu est refusé puis reconstruit

### B

- [ ] **`JobSystem`**
  - [ ] Test : 1 000 tâches s'exécutent toutes une seule fois
  - [ ] Test : l'arrêt se fait proprement, sans blocage
- [ ] **`IAsset`, `IAssetLoader` et `LoaderRegistry`**
  - [ ] Test : le bon loader est choisi selon le `TypeId` ; un type inconnu renvoie une erreur
- [ ] **`AssetHandle` et `AssetCache`**
  - [ ] Test : deux `Load` donnent la même instance
  - [ ] Test : l'asset est libéré quand le dernier handle disparaît
- [ ] **`AssetManager` : `Load`, `LoadAsync`, `Update`, `Unload`, `Save`**
  - [ ] Test : un `Load` pendant un `LoadAsync` ne charge qu'une fois
  - [ ] Test : 20 `LoadAsync` simultanés sans doublon ni blocage
  - [ ] Test : les callbacks s'exécutent sur le thread principal
  - [ ] Test : un fichier invalide donne l'état `Failed`
  - [ ] Test : relire l'asset sauvegardé avec `Save` donne le même objet

## Phase 4 : Interface et démo (18-19 nov., à deux)

- [ ] **Types de démo dans `sandbox/`** : `TextureAsset`, `MeshAsset`, `ConfigAsset`
  - [ ] Test : ils fonctionnent sans modifier `engine/`
- [ ] **Scénario complet dans le Sandbox** : créer, sauvegarder, chercher, charger
- [ ] **ImGui** : Asset Browser, Inspector, Archive Viewer, panneau Ressources
  - [ ] Vérification à l'œil, en suivant une liste de contrôle
- [ ] **Si le temps le permet :** chargement automatique des dépendances
  - [ ] Test : les dépendances se chargent avant l'asset ; un cycle est détecté

## Phase 5 : Rendu (19-20 nov.)

- [ ] **Passes de contrôle**
  - [ ] Tous les tests passent en Debug et en Release
  - [ ] AddressSanitizer pour les erreurs mémoire
  - [ ] Build et tests depuis un clone neuf
  - [ ] Supprimer les `.gitkeep`, `AssetEngine.cpp` et `Smoke.cpp`
- [ ] **Documents**
  - [ ] README et `ARCHITECTURE.md` à jour, diagramme à jour
  - [ ] Questions au prof réglées
- [ ] **Rendu**
  - [ ] Fusion de `dev` dans `main`
  - [ ] Migration vers le GitHub de l'école
- [ ] **Soutenance**
  - [ ] Slides, répétition en 15 minutes (3 + 5 + 5 + 2)
  - [ ] Revoir ensemble le code de l'autre pour le code review

## Le minimum à sécuriser

En cas de retard, ces éléments passent en priorité, car le cahier des charges les demande explicitement :

- les archives binaire et texte, avec tous leurs types ;
- le checksum et le HMAC ;
- le registry ;
- le chargement synchrone et asynchrone ;
- une interface ImGui simple.

Les dépendances entre assets, le versioning avancé et les finitions de l'éditeur peuvent sauter en premier.
