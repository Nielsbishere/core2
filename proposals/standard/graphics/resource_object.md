# Resource object

A resource object is a resource which can be held by a shader registry. This can be of 4 types; a Sampler, a GPUBuffer, a TextureObject or a GPUConstant.

## Texture object

A TextureObject is the representation of a buffer with a specific layout and is inherited by Texture (Texture1D, Texture2D, Texture3D), TextureArray (TextureArray1D, TextureArray2D, TextureArray3D) and TextureList (TextureList1D, TextureList2D, TextureList3D).

A [TextureList](texture_lists.md) is a number of textures within a TextureArray and requires both a uniform buffer binding and a texture binding.

A TextureArray is a Texture with layers and only requires a texture binding.

A Texture is a N-dimensional buffer with a stride specified by the type. It only requires a texture binding.

The texture object can be given to the ShaderRegistry object and this will internally bind the required resources.

### Depth object

Though it isn't an existing type, it is the combined name of the DepthTarget and the DepthTexture. The DepthTarget is only used while the RenderTarget is active and is removed after. The DepthTexture will maintain its data.

### RenderTexture

The render texture is used in the RenderTarget; a RenderTexture is a singular output color. Just like the vertex layouts, this uses "GPUFormat".

## Sampler

A sampler is a simple object that is used in combination with a texture to sample it. 

## GPUConstant

A GPUConstant is a little bit of data that is uniform for the entire shader. In Vulkan this is represented by a push constant, in OpenGL by a uniform. A GPUConstant contains a name, format, multi dimensional array indices, offset and a stride. The format is represented by "GPUConstantFormat". This is since it contains the ability to have matrices, which aren't supported by GPUFormat.

## GPUBuffer

A GPUBuffer contains the reflection alongside the data it has. This reflection is laid out like an array of GPUConstants. A GPUBuffer also has a type, a constant length and a stride. Normally the stride isn't used, though in cases of structured buffers (arrays of structs with undefined length) it is set to the size of that struct; and the buffer can be extended.

![ResourceObject UML](../uml/resource_object.png)