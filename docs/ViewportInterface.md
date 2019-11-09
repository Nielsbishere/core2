# ViewportInterface

A viewport interface is the developer-interface that handles everything that has to do with viewports.

This means that all important callbacks from low-level code (platform abstraction) is redirected to this interface. The interface is passed to a Window to a Viewport so it can be created.

The interface currently handles the following:

- Interface
  - constructor
  - destructor
- Viewport
  - init(ViewportInfo*)
  - release(const ViewportInfo*)
  - resize(const ViewportInfo*, Vec2u &size)
  - render(const ViewportInfo*)

All Viewport callbacks are done by the Viewport's thread.
The interface's creation is done by the thread that created it; this should preferably just be an object on the heap. Because then the thread that created it will also delete it.