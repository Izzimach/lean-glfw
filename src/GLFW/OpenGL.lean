
namespace OpenGL

@[extern "lean_opengl_glviewport"]
constant glViewport : Int → Int → Int → Int → IO Unit

inductive GLClearBufferEnum
| ClearColorBuffer
| ClearDepthBuffer
| ClearStencilBuffer

@[extern "lean_opengl_glclearbits"]
constant glClearBits : List GLClearBufferEnum → UInt32

@[extern "lean_opengl_glclear"]
constant glClear : UInt32 →  IO Unit

@[extern "lean_opengl_glclearcolor"]
constant glClearColor : Float → Float → Float → Float → IO Unit

end OpenGL