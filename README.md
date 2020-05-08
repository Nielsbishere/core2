# Core v2
![](https://github.com/Nielsbishere/core2/workflows/C%2FC++%20CI/badge.svg)

## Requirements

- CMake 3.13

## Installing Core v2

```bash
git clone --recurse-submodules -j8 https://github.com/Nielsbishere/core2
```

## Guide

These are the guides on various parts of core2:

- [ViewportInterface](docs/ViewportInterface.md)

## Virtual files

Virtual files are files that aren't present on disk but are present in the executable. In windows this means they are embedded in the exe, on Android it would be the apk. 

This can be used by the macro "add_virtual_files" included from the core2 cmake. This is an example of how to use it:

```cmake
add_virtual_files(
	DIRECTORY
		${CMAKE_CURRENT_SOURCE_DIR}/res
	NAME
		core2
	FILES
		shaders/test.frag
		shaders/test.vert
)
```

This would add the files F~/core2, F~/core2/shaders, f~/core2/shaders/test.frag, f~/core2/shaders/test.vert to the virtual file registry.

However, to access these, you have to add them to your final executable. This can be done by calling "include_virtual_files(myApp)". This function has to be called if other resources are included from other APIs, even if you don't use them.

