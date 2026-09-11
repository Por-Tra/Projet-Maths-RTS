# SFML Starter (Conan 2 + CMake, C++17)

Projet de démarrage minimal : C++17, SFML (graphique), gestion des
dépendances via Conan 2, build multiplateforme (Linux / Windows),
compatible Visual Studio 2022 (mode "Ouvrir un dossier" / CMake).

## Prérequis

- CMake >= 3.20
- Conan 2.x (`pip install conan`)
- Un compilateur C++17 : MSVC (VS2022), GCC ou Clang sur Linux
- Sur Linux, SFML a besoin de quelques libs système (X11, GL, etc.).
  Si Conan compile SFML depuis les sources (`--build=missing`), il
  peut demander l'installation de paquets système - suivez les
  messages de conan/apt si besoin (`libx11-dev`, `libudev-dev`,
  `libgl1-mesa-dev`, `libfreetype-dev`, ...).

Première utilisation de Conan sur une machine :
```bash
conan profile detect --force
```

## Build (identique sur Linux et Windows)

Depuis la racine du projet :

```bash
conan install . --build=missing -s build_type=Release
cmake --preset conan-release
cmake --build --preset conan-release
```

Conan génère automatiquement un `CMakeUserPresets.json` à la racine
(via `cmake_layout`), qui référence le toolchain généré dans
`build/Release/generators/`. Pas besoin d'éditer les presets à la main.

L'exécutable se trouve ensuite dans `build/Release/` (Linux) ou
`build/Release/Release/` selon le générateur utilisé (VS2022 est
multi-config).

Pour une build Debug, refaites simplement :
```bash
conan install . --build=missing -s build_type=Debug
cmake --build --preset conan-debug
```

## Utilisation avec Visual Studio 2022

1. Lancez `conan install . --build=missing -s build_type=Release`
   (et/ou Debug) une première fois en ligne de commande depuis le
   dossier du projet, avec le "Developer PowerShell" ou un terminal
   classique.
2. Dans VS2022 : **Fichier > Ouvrir > Dossier...** et sélectionnez
   le dossier du projet.
3. VS2022 détecte automatiquement `CMakeUserPresets.json` et propose
   les configurations `conan-release` / `conan-debug` dans le menu
   déroulant des configurations (en haut de la barre d'outils).
4. Sélectionnez `SfmlStarter.exe` comme élément de démarrage, puis
   compilez/lancez normalement (Ctrl+Shift+B / F5).

Les DLL nécessaires (SFML, etc.) sont copiées automatiquement à côté
de l'exécutable après compilation (voir `CMakeLists.txt`).

## Structure

```
sfml-conan-starter/
├── conanfile.py       # dépendances Conan (SFML 2.6.1)
├── CMakeLists.txt     # config CMake, C++17
├── src/
│   └── main.cpp       # fenêtre SFML de test (cercle + boucle d'event)
└── README.md
```

## Personnaliser

- Changer la version de SFML : éditer `self.requires("sfml/2.6.1")`
  dans `conanfile.py` (`conan search sfml -r conancenter` pour voir
  les versions disponibles).
- Ajouter d'autres dépendances Conan : ajouter une ligne
  `self.requires("xxx/version")` dans `requirements()`.

---

Compi linux : sudo apt install libx11-dev libxrandr-dev libudev-dev libgl1-mesa-dev libfreetype-dev


BUILD : 
cmake -B build
cmake --build build 
ou 
cmake --build build --config Release
