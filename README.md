# GARUDA
## GARUDA - Game And Rendering Unified Development Architecture

> GARUDA is a C++20 cross-platform game engine and development architecture built by and for "Studios SehajAvastha", engineered primarily for <ins>WINDOWS</ins> and <ins>ANDROID</ins>

---

## Architecture Overview

> The repository is divided into three strictly isolated top-level modules:

* **`Engine/` (`GarudaEngine`)** - The core C++20 static runtime library shared across all target platforms. Handles windowing, rendering, physics, math, asset loading, and debug UI.
* **`Project/`** - The game-specific/project-specific runtime wrapper and packaging layer. Contains platform entry points and per-project deployment configurations (including Gradle, `AndroidManifest.xml`, and dynamic `GARUDA_PACKAGE_NAME` injection for Android APKs).
* **`Editor/` (`GarudaEditor`)** - Desktop-only development tooling (Windows) built on top of `GarudaEngine` for scene editing, asset management, and project packaging.

---

## Repository Structure

```text
GARUDA/
├── CMakeLists.txt           # Root C++20 build configuration
├── .clang-tidy              # Static analysis & naming convention rules
├── cmake/
│   └── Dependencies.cmake   # Automated dependency fetching & GENie build integration
├── Engine/
│   ├── Include/Garuda/      # Public engine API headers
│   └── Source/              # Core engine implementation
├── Project/
│   ├── Source/              # Game runtime entry point
│   └── Android/             # Gradle project, GarudaActivity.java, and AndroidManifest.xml
└── Editor/
    ├── Include/             # Editor UI & tooling headers
    └── Source/              # Desktop editor implementation
```

## Building GARUDA

### Prerequisites
* **C++20 Compiler** (MinGW-s64 GCC or MSVC on Windows; Android NDK r27d+ for Android)
* **CMake 3.24+** & **Ninja / Make**
* **JDK 17** & **Android SDK** (for Android builds)

### Windows (Desktop Runtime & Editor)

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```
Binaries are output to `build/bin/`.

### Android (`arm64-v8a`)
Gradle invokes the root `CMakeLists.txt` automatically via `externalNativeBuild` and packages the native `libmain.so` alongside SDL3's Java bridge:

```bash
cd Project/Android
./gradlew installDebug -PpackageName=com.sehajavastha.garuda
```

---

## License
See `LICENSE` for details.

---

# Libraries Used

## 1. SDL3
![SDL_Logo](https://external-preview.redd.it/trying-out-sdl3-by-writing-a-c-game-engine-v0-kvtxrU34YX9xvD3i0fIZ-nFa00MS9oL8H_7bIouw1TA.png?width=640&crop=smart&auto=webp&s=16fcb001d5b0bfb12dd7e176a1056f8a84979ef2)

## 2. BGFX w/ GENie (bx + bimg)
![BGFX_Logo](https://encrypted-tbn0.gstatic.com/images?q=tbn:ANd9GcT9y0ORwsPKRPSsO5tOa6R2bINXcqDewJA6lbMyyvqIJy0I1BRSqrYE8Rs&s=10)

## 3. JoltPhysics
![Jolt_Logo](data:image/jpeg;base64,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)

## 4. Box2D
![Box2D_Logo](data:image/png;base64,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)

## 5. GLM
![GLM_Logo](https://encrypted-tbn0.gstatic.com/images?q=tbn:ANd9GcQAisDBd5sxoBIyd3TgL94xKkd5565PUdRvoGZFxmMj3A&s=10)

## 6. STB

## 7. Dear Imgui
![ImGui_Logo](https://encrypted-tbn0.gstatic.com/images?q=tbn:ANd9GcRS88tb68AfycYZZibqm8TQFkU-3KMlxui8rKcyqST8GQ&s=10)

## 8. ENet