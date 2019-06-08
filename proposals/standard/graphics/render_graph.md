# Render graph

A render graph is a combination of render tasks that is executed to render to the output.

## Render task

A render task is a combination of graphics objects, including: a shader registry and a render target.

This render task transitions the input render targets to be used. Each dependency has a shader stage related to when it has to be used. 

```cpp
renderGraph->add<MyTask, MyPreprocessTask>(RenderTaskInfo(RenderTaskType::GRAPHICS, myShader), { "myPreprocessTask", ShaderStageType::FRAGMENT });
```

The example above adds "MyTask" (with the shader and shader registry of 'myShader' and type GRAPHICS (rasterizer)) to the render graph with the dependency MyPreprocessTask. The created render task will wait for MyPreprocessTask at the execution of the fragment shader. It will put this into the shader registry at "myPreprocessTask".

Every render task internally handles resizing, destruction and updating the shader registry based on these changes automatically. Only allocations done by the user have to be taken care of by them.

### Types

There are 4 types of render tasks; graphics (anything fixed-pipeline related), compute, raytracing and copy. 

These have varying levels of support that can be queried for as well with the Graphics class.