# Architecture d'Asset Engine

Ce document fixe les règles communes du projet. Si une décision change, on le met à jour dans le même commit que le code concerné.

## Modules et dépendances

```text
Sandbox / Editor / Tests
          ↓
        Asset
       ↓     ↓
Serialization   Security
       ↓     ↓
        Core
```

Une dépendance ne va que vers le bas. `Serialization` et `Security` ne s'incluent jamais l'une l'autre : elles se rencontrent à travers les streams, et c'est `Asset` qui les assemble.

| Module | Contenu |
|---|---|
| `Core` | Streams (`IO/`), réflexion (`Reflection/`), système de fichiers, logs, `Result`, hash |
| `Serialization` | `IArchive`, `Serializer<T>`, archives binaire et texte, lib JSON (`Json/`) |
| `Security` | `IIntegrityProvider`, CRC32, SHA-256, HMAC, `SecurityPolicy`, `SecureStream` |
| `Asset` | `AssetManager`, `AssetRegistry`, `AssetHandle`, `AssetCache`, loaders, `JobSystem` |

Le moteur ne contient aucun type propre à une démo : `TextureAsset`, `MeshAsset` et `ConfigAsset` vont dans `sandbox/`.

## Sérialisation

- `IArchive` est abstraite. Une seule fonction `Serialize` sert au chargement et à la sauvegarde.
- `IsLoading()` et `IsSaving()` ne servent que dans les cas asymétriques.
- Chaque archive concrète expose aussi `static constexpr bool IsLoading`.
- `Serializer<T>` choisit le traitement avec des concepts C++20.
- En binaire, les tailles sont écrites en varint.
- En JSON, une map à clé non textuelle s'écrit comme un tableau de paires, et chaque classe de base a son propre sous-objet.
- La lib JSON est écrite par nous.

## Réflexion

- Un `TypeId` est le hash du nom déclaré, calculé à la compilation. On n'utilise jamais `typeid().hash_code()`, qui n'est pas stable.
- Le nom, la version, les champs et les bases sont déclarés au niveau du type, sans instance.
- Un type a une liste de bases, avec une fonction de conversion par base. On n'utilise jamais `reinterpret_cast` pour passer à une base.
- Une base virtuelle n'est sérialisée qu'une fois.
- Les types sont enregistrés avant `main`, puis le registre est gelé : enregistrer un type ensuite est une erreur. Les threads peuvent donc le lire sans mutex.
- `AssetEngine` reste une `OBJECT` library, sinon le linker pourrait retirer ces enregistrements.

## Format `.asset`

| Bloc | Contenu |
|---|---|
| Header | Magic `AENG`, version du format, encodage (binaire ou texte), `TypeId`, taille des métadonnées, taille du payload |
| Metadata | Infos lues par le registry : version, dépendances, tags |
| Payload | Données de l'objet |
| Footer | Digest du header, des métadonnées et du payload |

Le registry lit le header et les métadonnées, puis saute le payload sans le charger.

En texte, le fichier contient un objet JSON avec les clés `header`, `metadata` et `payload`, suivi d'une dernière ligne avec le digest en hexadécimal. Le digest porte sur les octets exacts qui précèdent cette ligne.

## Sécurité

| Contexte | Algorithme | Protège contre |
|---|---|---|
| Debug, cache, `registry.bin` | CRC32 | Corruption accidentelle |
| Assets en Release | HMAC-SHA256 | Corruption et modification volontaire |
| Sauvegardes | HMAC-SHA256 avec clé propre à l'utilisateur | Idem, et copie entre utilisateurs |

- `SecurityPolicy` impose l'algorithme selon le contexte. Il n'est jamais lu dans le fichier : sinon, un attaquant pourrait demander un CRC32 et le recalculer.
- Le digest est vérifié avant de désérialiser, et comparé en temps constant.
- Une clé embarquée dans un exécutable reste récupérable : le HMAC complique la falsification, il ne la rend pas impossible.

## Assets et chargement

- `AssetId` est le hash du chemin normalisé. `Scan()` signale les collisions.
- `registry.bin` est un cache : la référence reste les fichiers `.asset`, et `Scan()` le reconstruit.
- Une entrée du cache est créée en état `Loading` avant le lancement du job. Un second `Load` ou `LoadAsync` attend donc ce chargement au lieu d'en relancer un.
- Les callbacks s'exécutent sur le thread principal, dans `AssetManager::Update()`.
- Un job n'appelle jamais `Wait()`, ce qui pourrait bloquer tout le pool.
- Le chargement automatique des dépendances n'est prévu que s'il reste du temps. Elles sont déjà déclarées dans les métadonnées.

## Conventions

- **Noms :** `PascalCase` pour les types et les fonctions, préfixe `I` pour les interfaces, `m_` pour les membres.
- **Fichiers :** un type principal par fichier, avec le `.h` dans `engine/include/AssetEngine/<Module>/` et le `.cpp` dans `engine/src/<Module>/`.
- **Tests :** chaque ajout au moteur a son test dans `tests/<Module>/`.
- **Commits :** en anglais, au format `type : description` (`feat`, `fix`, `docs`, `chore`, `refactor`).
- **Branches :** une branche par personne, fusionnée dans `dev`. `main` ne reçoit que ce qui est stable.
- **Dépendances externes :** Catch2 et ImGui, récupérées par CMake et jamais versionnées.

## Répartition

| | Personne A | Personne B |
|---|---|---|
| **Pour commencer** | Réflexion | Lib JSON, CRC32, SHA-256, HMAC |
| **Ensuite** | `IArchive`, `BinaryArchive`, `Serializer<T>` | `TextArchive`, `SecureStream` |
| **À la fin** | `AssetRegistry`, format `.asset` | `JobSystem`, `AssetCache`, `AssetHandle`, `AssetManager` |

À deux : les streams au départ, puis ImGui, la démo et la soutenance.

A pousse `IArchive` sur `dev` dès qu'elle est stable, sans attendre `BinaryArchive`.

## À valider

- [ ] Qui est A, qui est B
- [ ] Le détail du format `.asset`
- [ ] « Paramètres statiques » : nous avons retenu les métadonnées de type et la détection load/save à la compilation, à confirmer avec le prof
- [ ] « Enregistré avant le runtime » : nous avons retenu les types, enregistrés avant `main`, à confirmer avec le prof
- [ ] La phrase incomplète du cahier des charges, « l'optimisation de mise »
