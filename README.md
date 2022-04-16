# lean-glfw

Declarations and C thunks to allow access to [GLFW](https://glfw.org) and [OpenGL](https://www.khronos.org/opengl/)
from the [lean4](https://github.com/leanprover/lean4) theorem prover.

Emphasis is on using the bindless or [Direct State Access](https://www.khronos.org/opengl/wiki/Direct_State_Access) method introduced in OpenGL 4.5.  This means that binding
objects and modifying "the current bound xxx" is discouraged and you should instead use functions that explicitly specify the
buffer/VAO/program you are modifying.

# building

To build and run this package you need access to both GLFW and it's dependencies. On windows this includes GDI32 which is
provided by the Windows SDK.

Once you have these items installed you need to edit the [lakefile](./lakefile.lean) to point to both GLFW and the
Windows SDK.

At this point "lake build" should (hopefully) work.
