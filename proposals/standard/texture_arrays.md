# Texture arrays

**NOTE: Even though the concept uses OpenGL terminology, it works on all other APIs as well.**

A huge problem with texture arrays is that OpenGL ES3 doesn't support arrays of textures. This means that having >16 differently sized textures is not supported. However, Texture2DArray is supported.

To avoid having to allocate to much, we can use either texture compression or sparse textures. Sparse textures don't work on web, so in that case the only option is to allocate a compressed Texture2DArray.

```cpp
struct TextureRegion {
    u32 xy;			//uint; upper half x, lower half y
    u32 zlayer;		//z, layer
    u32 wh;			//width, height
    u32 lmips;		//length, mips
};

layout(std140) uniform TextureRegions {
    TextureRegion regions[4096];
}

TextureArray2D array;	//4096x4096xN + 2048x2048xN + ...
```

This texture object has a stride of 16 and can be used in a UBO. This means that we support 4096 textures bound at the same time (4 KiB * 16 = 64 KiB = minimum size of a UBO). Our TextureArray can suballocate TextureRegions into its texture layers. This means that a 4k image can use up the entirety of 1 layer, but 1 layer can also be used for 4 2k textures, 16 1k textures, 64 512x textures, etc.
These have to be the same format, so no HDR/SRGBA mixes, one or the other.

This requires a 2D allocator that can allocate the mips and takes into account alignment (4x4 blocks since compression). If we go with 4k per layer, we need 12 mip maps. This means that we can't use anisotropic filtering and we will use:

64 MiB + 16 MiB + 4 MiB + 1 MiB + ... = 85,... MiB per layer without compression.

With ETC2 it is expected to be compressed 6x, resulting in 14,... MiB per layer.

This means we can't rely on the default mip generation and prediction and we need our own functions. 

```cpp
float getMipLevel(vec2 uv, vec2 textureSize) {
    vec2 textureLoc = uv * textureSize;
    vec2 dxVtc = dFdx(textureLoc);
    vec2 dyVtc = dFdy(textureLoc);
    float deltaMaxSqr = max(dot(dxVtc, dxVtc), dot(dyVtc, dyVtc));
    float mml = 0.5 * log2(deltaMaxSqr);
    return max(0, mml);
}
```

The mips should be pre-baked into ETC2 (mobile/console) and S3TC (desktop) format. These can then be forwarded to the GPU and instead of making the mips it can copy them.

glTexSubImage3D allows you to set the data per region, mip and layer.

## Variables for memory usage

The memory usage for the approach is as following:
$$
textureRegions * 16 + layers * 4/4 * \sum^{⌊\log_2{textureSize}⌋ }_{n=0}textureSize^2 * 4^{-n}
$$
Meaning that for textureSize 4096 with 32 layers, this would require 12 mips. And using 4096 texture regions:

4096 * 16 + 32 * (16777216 + 4194304 + 1048576 + 262144 + 65536 + 16384 + 4096 + 1024 + 256 + 64 + 16 + 4 + 1) =

64 KiB + 32 * 22369621= 64 KiB + 699 050  KiB = 682.7 MiB = 0.6667 GiB.

## Binding a texture array

It takes 2 bindings, the uniform buffer binding for the texture regions and the texture binding.

## Sparse textures



