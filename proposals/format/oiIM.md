# oiIM

The oiIM format is a way of submitting the images to the GPU without much CPU intervention. The image contains all required mips, can hold compressed and uncompressed textures. It supports 1D, 2D and 3D images. Besides mips, this format also supports multiple layers, so you can export your multi layered 2D image to oiIM.

oiIM can be converted to and from other formats like .png, .jpg, .hdr, .dds, .ppm, etc. 

## File spec

## Image header

```cpp
//32B
struct Header {
  
    u8 magicNumber[4];		//oiIM
    
    u16 version;			//>0
    u16 flags;				//See flags
    
    u16 width;				//>0
    u16 height;				//>0
    
    u16 depth;				//>0
    u8 format;				//Texture format (see texture.md)
    u8 p0 : 2;				//Padding (0)
    u8 dims : 2;			//Invalid, 1D, 2D, 3D
    u8 mips : 4;			//+1 = Supporting up to 16 mips
    
    u16 regions;			//The number of regions in this texture
    u16 layers;				//>0
    
    u64 flagParam0;			//Currently used by USE_COMPRESSION flag.
        
    u32 flagParam1;			//Reserved for future use (0)
    
};
```

### Flags

Currently there's only one additional flag; USE_COMPRESSION (1). If this is turned on, it uses zlib compression to compress all data to save disk space. This approximately saves 50% disk space. However, it should be noted that using a compressed texture format should already save a lot of space and compressing that even more will cause potential slow downs as well as CPU usage. 

## Regions

A "TextureRegion" is a subdivision of the texture that can be used for other purposes. This could be a TileMap with differently sized textures, a sub-allocated texture, etc.

Regions aren't required, though if there's no regions available it will create a default region (offset 0, length max, name="").

```cpp
struct Region {
    
    u16 x, y;
    
    u16 z, layer;
    
    u16 width, height;
    
    u16 length, name;
    
};
```

Where name is a valid index into a oiSL file. Each region gets the default mips to be allocated `floor(log(x) / log(2)) + 1` where x is the biggest from width/height or length. 

The region has to be in bounds, meaning that x + width <= textureWidth, y + height <= textureHeight, z + length <= textureLength.

Regions may not overlap, though in theory it could be possible. 

## Image data

With the format, layers, width, height, depth, it can calculate the buffer that contains all of the texture data. Using the following pseudo code

```
calculateFileSize(width, height, depth, layers, mips, format) {
    
    val = 0
    
    foreach mip
                
    	blockX = ceil(mipWidth / compressionBlockX)
    	blockY = ceil(mipHeight / compressionBlockY)
    
    	val += blockX * blockY * blockSize * mipDepth
    	
    ret val
    
}
```

If compression is not supported, compressionBlockX and Y return 1. blockSize is the size per block in bytes (4 for RGBA with 1x1 block dims). Check texture.md for all texture formats and their values.

For a 4k image with compressed texture format 13 mips and 32 layers, the layout would be:

```cpp
struct {
	unsigned char mip0[16][1024][1024][1];	//4k
	unsigned char mip1[16][512][512][1];	//2k
	unsigned char mip2[16][256][256][1];	//1k
	unsigned char mip3[16][128][128][1];	//512
	unsigned char mip4[16][64][64][1];		//256
	unsigned char mip5[16][32][32][1];		//128
	unsigned char mip6[16][16][16][1];		//64
	unsigned char mip7[16][8][8][1];		//32
	unsigned char mip8[16][4][4][1];		//16
	unsigned char mip9[16][2][2][1];		//8
	unsigned char mip10[16][1][1][1];		//4
	unsigned char mip11[16][1][1][1];		//2
	unsigned char mip12[16][1][1][1];		//1
} layers[32];
```

Giving it a size of 32 * (16 777 216‬ + 4 194 304 + 1 048 576 + 262 144‬ + 65 536 + 16 384 + 4 096‬ + 1 024 + 256 + 64 + 16 + 16 + 16) = 32 * 22 369 648‬ = 715 828 736 = 0.666667 GiB.