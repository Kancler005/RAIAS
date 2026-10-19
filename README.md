# RAI: Ashes of Eternity

**RAI: Ashes of Eternity** is a dark-fantasy Action RPG / MMORPG prototype built on a custom C++ engine.

The project experiments with a hybrid rendering style:

- real 3D geometry;
- lighting and depth;
- isometric camera;
- high-detail colored ASCII post-processing;
- world-space glyph rendering;
- procedural environment meshes;
- an in-engine scene editor.

Current version: **v0.0.27 — Animated Knight Player**

> Status: **Pre-alpha**

---

## Current Visual Style

RAI is not a terminal game.

The world is rendered as true 3D geometry and then converted into a colored ASCII image.
<img width="2156" height="1422" alt="image" src="https://github.com/user-attachments/assets/515f2ef0-3714-4836-8091-fa186b7c27e6" />

Current pipeline:

```text
3D World
    ↓
Lighting / Depth / Materials
    ↓
Hi-Detail ASCII Postprocess
    ↓
Directional Outline Glyphs
    ↓
World-space Glyph Layer
    ↓
Editor UI
```

The renderer can use symbols such as:

```text
. : - + * # @
/ \ | ( ) [ ] { } _ =
, ' ^ % ~
```

Directional edges are used to make objects more readable:

```text
vertical edge     -> |
left/right curve  -> ( )
hard border       -> [ ]
strong border     -> { }
diagonal edge     -> / \
horizontal edge   -> _ =
```

---

## v0.0.27 — Animated Knight

The old player cube has been replaced with a modular animated knight.

The character currently has:

- helmet;
- visor;
- crest;
- armored torso;
- shoulder armor;
- arms;
- legs;
- boots;
- tabard;
- cape;
- sword;
- shield;
- belt.

### Animation

The knight has two runtime animation states.

**Idle**

- breathing;
- subtle body motion;
- cape sway.

**Movement**

- alternating leg movement;
- opposite arm movement;
- sword follows the weapon arm;
- shield follows the off-hand;
- torso rotation;
- body bob;
- stronger cape sway;
- smooth rotation toward the actual movement direction.

Movement animation is driven by the player's real displacement after collision resolution.

---

## World

The current test world includes:

- expanded playable territory;
- procedural rocks;
- 3 rock variants;
- ruined shrine;
- broken watchtower;
- dead trees;
- campfires;
- animated fire glyphs;
- sparks;
- moving grass;
- ash / dead-grass / stone glyph materials;
- dark atmospheric haze.

The player movement area is currently approximately:

```text
X: -17 ... +17
Z: -17 ... +17
```

---

## Editor

Press:

```text
F1
```

to enter Editor Mode.

Current editor features:

- Hierarchy;
- Inspector;
- mouse selection;
- Move gizmo;
- Rotate gizmo;
- Scale gizmo;
- World / Local transform mode;
- create objects;
- duplicate objects;
- delete objects;
- scene save/load.

### Editor Controls

```text
F1              Editor Mode
W               Move gizmo
E               Rotate gizmo
R               Scale gizmo

TAB             Next entity
Shift + TAB     Previous entity

N               Create cube
Cmd/Ctrl + D    Duplicate
Delete          Delete selected entity

F5              Save scene
F9              Reload scene
```

The Player cannot be accidentally duplicated or deleted through the editor.

---

## Gameplay Controls

```text
W A S D         Move
F1              Editor Mode
F5              Save scene
F9              Reload scene
```

Enemy AI currently exists as an early prototype.

Combat is planned for later milestones.

---

## Engine

The engine is written in **C++20**.

Main technologies:

- SDL3
- SDL_GPU
- Metal on macOS
- GLM
- CMake
- Ninja
- Dear ImGui
- ImGuizmo
- nlohmann/json
- cgltf (experimental model-import path)

Primary development platform:

```text
macOS
Apple Silicon / arm64
Metal
```

A Windows/D3D12 path is planned.

---

## Source Structure

```text
src/
├── main.cpp
├── engine/
│   ├── Application.*
│   ├── Renderer.*
│   ├── Camera.h
│   ├── physics/
│   │   └── CollisionSystem.*
│   ├── render/
│   │   ├── Mesh.*
│   │   ├── GlyphRenderer.*
│   │   ├── ProceduralMeshes.*
│   │   └── ModelLoader.*
│   └── scene/
│       ├── Transform.h
│       ├── Entity.h
│       ├── Scene.*
│       ├── SceneSerializer.*
│       └── WorldExpansion.*
├── editor/
│   └── EditorUI.*
└── game/
    ├── GameWorld.*
    ├── GameState.h
    └── EnemySystem.*
```

---

## Building

The current development setup uses CLion + CMake + Ninja.

Typical workflow:

```bash
cd ~/CLionProjects/RAIAS
cmake --build cmake-build-debug
./cmake-build-debug/RAI
```

The game expects its runtime assets relative to the project working directory.

Recommended CLion working directory:

```text
$ProjectFileDir$
```

---

## Scene

The current development scene is stored in:

```text
assets/worlds/test_world.json
```

Scene entities currently support:

- unique ID;
- name;
- prefab name;
- position;
- rotation;
- scale;
- color;
- visibility;
- collision flag.

Runtime-only editor and animation fields are intentionally not serialized.

---

## Project Roadmap

Near-term priorities:

```text
v0.0.28
Enemy / creature presentation

v0.0.29
Attack animation + melee combat foundation

v0.0.30
Hit reactions + damage + combat VFX

later
Asset Browser
Prefabs
Improved model importer
World tools
Water glyph renderer
Particles
Dungeons
Loot
Abilities
Co-op / multiplayer
Persistence
Guilds / trade
```

The version order may change as the renderer and editor evolve.

---

## Release History

### v0.0.27

Animated knight player.

### v0.0.26

Dead trees, grass wind, campfires, sparks and atmosphere.

### v0.0.25

Hi-Detail ASCII renderer and directional outline glyphs.

### v0.0.24

Expanded world, procedural rocks and structures.

### v0.0.21

Transform gizmos and graphical editor workflow.

### v0.0.20

Hierarchy and Inspector with Dear ImGui.

### v0.0.15

First public pre-alpha milestone.

---

## License

No public license has been selected yet.

Unless a license is added later, source code and game assets remain under the copyright of the project owner.

---

## Development Status

RAI is under active development.

Expect:

- unfinished systems;
- experimental renderer changes;
- temporary assets;
- breaking changes between pre-alpha versions.

The goal is to turn the current prototype into a dark-fantasy world where real 3D geometry, animation and gameplay are presented through a distinctive colored ASCII visual language.
