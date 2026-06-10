# Tinyrenderer
A real-time software renderer in C++ built by following [ssloy's tinyrenderer course](https://haqr.eu/tinyrenderer/), extended with interactive rendering via Raylib, parallel rasterization using OpenMP, and an ImGui control panel for manipulating the scene at runtime.

## Building

### Linux & macOS
```bash
git clone https://github.com/aaronboch/tinyrenderer
cd tinyrenderer
cmake -B build/release && cmake --build build/release -j
```

Or use the Zed task (`build-project`) if you're editing in Zed.

### Windows
Not tested — the build system should work with MSVC or MinGW, but you may hit issues.

### Dependencies
- CMake (used for building)
- raylib (used for window management and input handling)
- **Optional:** OpenMP (recommended for faster rendering)

## Usage 
```bash
./build/release/tinyrenderer
```

Or use the Zed task (`run-project`).

## Features
### From Course
- [x] Model loading
- [x] Rasterization
- [x] Phong reflection model 
  - [x] On faces
  - [x] On vertices
- [x] Normal mapping
- [x] Texture mapping
- [ ] Shadow mapping
- [ ] Ambient occlusion

## My additions
- [x] Raylib instead of rendering out to one image.
- [x] Performance optimizations (to allow real time rendering)
  - [x] Parallel face loop instead of small part in rasterize function.
  - [x] General performance improvements
  - [x] Backface culling
  - [x] Object-level frustum culling
  - [ ] Blinn-Phong reflection model
- [x] Controlling Camera
- [x] Loading Multiple Models 
  - [x] Support for multiple models in the same scene (e.g offsetting postion of each model, etc.)
- [ ] ImGui to Control variables (e.g. Light direction, Roughness, etc.)
  - [x] Basic ImGui Setup
  - [x] Adding ability to load new models from ImGui 
  - [ ] Adding controls for variables
    - [x] Model position/rotation/scale
    - [ ] Shader properties (e.g. Light direction, Roughness, etc.)
    - [x] Camera position/rotation etc.

## License
MIT
