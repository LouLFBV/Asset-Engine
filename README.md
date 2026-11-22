# Asset-Engine

Système d'archive binaire et texte, avec contrôle d'intégrité et moteur d'assets, en C++20.

## Compiler et tester

Il faut Visual Studio 2026 et CMake 3.25 ou plus. La première configuration demande un accès à Internet pour récupérer Catch2.

```bash
cmake --preset vs2026
cmake --build --preset debug
ctest --preset debug
```

Ensuite rendez vous sur : build/AssetEngine.slnx
Et vous pourrez par la suite définir "Editor" ou "ProjetAssetEngine" comme projet de démarrage pour tester l'asset engine de Jupiter ou Lou respectivement. 

## Structure

- `engine/` : la bibliothèque
- `tests/` : les tests automatiques
- `sandbox/` : le programme d'essai et de démo
- `Docs/ARCHITECTURE.md` : les règles et décisions du projet
