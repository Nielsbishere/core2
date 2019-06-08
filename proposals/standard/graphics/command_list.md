# Command list

The command list class is a wrapper for the low-level implementation. Since OpenGL doesn't have a command list, this will just forward the calls to the driver. A command list is abstracted in such a way that there's an array of instructions that are compatible for most APIs. Each command inherits a Command and has a suffix if it is not standard. 

| Name      | Suffix | Description                                                  |
| --------- | ------ | ------------------------------------------------------------ |
| Feature   | Ft     | A feature that has to be supported by the API (FtA) & GPU (FtG); including compute shaders, raytracing, VR, etc. |
| Extension | Ext    | An extension that can be enabled on per driver basis.        |

If the command list is not updated, it won't have to recreate the command lists. The implementation can decide how it fills the api-dependent command buffers (multi-threading, etc.).

The command list is filled on the main thread and executed on a separate thread.