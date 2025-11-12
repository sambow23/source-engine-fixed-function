# D3D8 Fixed Function Shader API Backend

## Overview

This is a Direct3D 8-style fixed function rendering backend for Source Engine, specifically designed for **NVIDIA RTX Remix** compatibility. It uses D3D9 devices but only calls fixed function pipeline APIs, allowing RTX Remix to intercept and enhance the rendering with ray tracing.

## Purpose

RTX Remix works by intercepting D3D8/D3D9 fixed function calls and replacing them with ray-traced equivalents. Source Engine normally uses programmable shaders (Shader Model 2.0+), which RTX Remix cannot intercept. This backend translates Source's shader-based materials into fixed function pipeline states.

## Features

- **RTX Remix Compatible**: Uses only D3D9 fixed function calls that RTX Remix can intercept
- **DXVK-Native Support**: Works with DXVK-native on Linux
- **Automatic Material Fallbacks**: Translates shader-based materials to fixed function equivalents
- **Fixed Function Lighting**: Up to 8 simultaneous lights using D3D fixed function pipeline
- **Multitexturing**: Supports up to 8 texture stages for blending operations

## Usage

### Command Line Flags

Launch the game with either of these flags:

```bash
# Use the D3D8FF backend
./hl2.sh -d3d8ff

# Or use the RTX Remix alias
./hl2.sh -rtxremix
```

### With RTX Remix

1. Build and install the D3D8FF backend
2. Launch the game with `-rtxremix` flag
3. Inject RTX Remix using your preferred method
4. RTX Remix will intercept the fixed function calls and apply ray tracing

## Limitations

### Not Supported

- **Programmable Shaders**: All shader-based effects are translated to simple approximations
- **HDR Rendering**: Requires shader support
- **Advanced Material Effects**: Bump mapping, parallax, complex lighting models
- **Post-Processing**: Bloom, depth of field, motion blur, etc.

### Supported

- **Basic World Rendering**: Geometry with textures and lightmaps
- **Vertex Lighting**: Simple lighting on models
- **Translucency**: Alpha blending for transparent surfaces
- **Fog**: Distance-based fog
- **Multitexture Blending**: Up to 8 texture stages

## Technical Details

### Architecture

The backend consists of:

- `IShaderDeviceMgr` - Adapter enumeration and device creation
- `IShaderDevice` - Device lifecycle management
- `IShaderAPI` - Rendering state and draw calls
- `IShaderShadow` - State snapshot management
- `IHardwareConfigInternal` - Capability reporting

### Fixed Function State Mapping

- **Texture Stages**: Uses `SetTextureStageState()` for texture blending
- **Lighting**: Uses `SetLight()` and `LightEnable()` for scene lighting
- **Materials**: Uses `SetMaterial()` for surface properties
- **Fog**: Uses fixed function fog states
- **Alpha Blending**: Uses `SetRenderState()` for blend modes

### DX Level

Reports as DX 8.0 (level 80) to force materials to use fixed function fallbacks.

## Building

### With waf

The module is automatically built as part of the main Source Engine build:

```bash
./waf configure
./waf build
```

### Output

The build produces `shaderapid3d8ff.so` (Linux) or `shaderapid3d8ff.dll` (Windows) in the game's `bin/` directory.

## Development Status

### Completed

- [x] Device management and initialization
- [x] Hardware capability reporting
- [x] Build system integration
- [x] Command line flag support

### TODO

- [ ] Texture stage state management
- [ ] Fixed function lighting implementation
- [ ] Mesh rendering with FVF
- [ ] Material shader fallback system
- [ ] State snapshot management
- [ ] Vertex/index buffer management

## Compatibility

- **Windows**: Full support with D3D9
- **Linux**: Works with DXVK-native
- **RTX Remix**: Primary target platform

## License

Same as Source Engine - see main LICENSE file.

## Credits

Based on Valve's shaderapidx9 architecture, adapted for fixed function rendering.

