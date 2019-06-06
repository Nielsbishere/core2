# OCore v2

## Requirements

Linux shell (Windows users can use GitBash)

## Installing OCore

```bash
git clone --recurse-submodules -j8 https://github.com/Oxsomi/core2
```

## Setting up an environment

The environment is what platforms you currently build for. You can add the same platform under different names and settings.

### Automatic

If you run build.sh without arguments, it will help you set-up your environment and allow you to select one or add a new one. If an environment is set, you can build it.

### Command arguments

`-name=x` The name of the environment (case sensitive, underscores instead of spaces)

`-type=x` The environment type (Android, Windows, Linux, OSX, Web)

`-arch=all` The architecture type (ARM32, ARM64, x86, x64, all, 64_bit) can use multiple unless it's all/64_bit. Unused for web

`-bake=only` How to use the baker (only, on, off); only (exclude non-baked files), on (includes both baked and non-baked files), off (leave all resources as-is)

`-log=file` How log calls are handled; console, file, off

`-gapi=x` The API used for graphics; default is Vulkan, fallback is OpenGL (4.5 & ES3). 

`--debug` Enable debug mode; defaulting `bake=on`, `log=console` and optimizations to be limited for debugging purposes. Disabled by default

`-f<x>` All tags starting with 'f' are passed along to CMake and can be used by the end-user

`--help` Will show all command line arguments. If you specify the type, it will give you specific info about that environment type

#### Building

Once an environment is created, you can build it. This is done with the following parameters:

`-env=x` The environments you want to build. Can't be used while creating an environment

`-threads=8` The number of threads you want to use

#### Baking

If baking is enabled (highly recommended), you can specify the following parameters:

`-maxResolution=0` The resolution limit of every texture for this build; if you supply 4k textures but want an Android build, you can limit this to anything you want. Setting this to zero will allow any texture size

`--runtime_shader_compilation` Will add support for runtime shader compilation (disabled by default), normally this is done by the baker by converting the shaders to the target language (HLSL/GLSL/SPV)

#### Android

`-api=i` The Android api level

## Baking

The baker is a tool that will allow you to save a lot of memory, bandwidth and conversion struggles. It converts to oi formats, which can be converted into other formats.

The baking system allows you to write HLSL/GLSL shaders and transform them into SPV/HLSL/GLSL, as well as allowing to limit texture size and compressing textures & models.