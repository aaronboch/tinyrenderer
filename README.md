# Tinyrenderer
Simple software renderer in C++ using raylib for window management and input handling.
The renderer is based on Dmitry V. Sokolovs tinyrenderer hands-on course, which can be found here:
- https://haqr.eu/tinyrenderer/
- https://github.com/ssloy/tinyrenderer

## Building

```bash
git clone https://github.com/aaronboch/tinyrenderer
cd tinyrenderer
cmake -B build/release && cmake --build build/release -j
```

Or use the Zed task (`build-project`) if you're editing in Zed.

### Dependencies
- CMake (used for building)
- raylib (used for window management and input handling)
- **Optional:** OpenMP (recommended for faster rendering)

## Usage 
```bash
./build/release/tinyrenderer [path/to/model].obj
```

Or use the Zed task (`run-project`), which fuzzy-finds `.obj` files and prompts you to pick one.

## Features
- [x] Model loading
- [x] Rasterization
- [ ] Phong reflection model 
  - [x] On faces
  - [ ] On vertices
- [ ] Normal mapping
- [ ] Texture mapping
- [ ] Shadow mapping
- [ ] Ambient occlusion

## License
MIT — based on [ssloy/tinyrenderer](https://github.com/ssloy/tinyrenderer) (also MIT).
