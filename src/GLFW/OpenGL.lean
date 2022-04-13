
namespace OpenGL

--
-- render start/setup funcs
--

@[extern "lean_opengl_glviewport"]
constant glViewport : Int → Int → Int → Int → IO Unit

inductive ClearBufferEnum
| ClearColorBuffer
| ClearDepthBuffer
| ClearStencilBuffer

-- given a list of clear buffers finds the value for passing into glClear
@[extern "lean_opengl_glclearbits"]
constant glClearBits : List ClearBufferEnum → UInt64

-- call glClearBits to find the value you should pass in for the UInt64
@[extern "lean_opengl_glclear"]
constant glClear : UInt64 →  IO Unit

@[extern "lean_opengl_glclearcolor"]
constant glClearColor : Float → Float → Float → Float → IO Unit

--
-- Buffers
--

def GLBufferObject := UInt32

instance : ToString GLBufferObject where
    toString x := let (y : UInt32) := x
                  toString y

@[extern "lean_opengl_glgenbuffers"]
constant glGenBuffers : UInt32 → IO (List GLBufferObject)

@[extern "lean_opengl_gldeletebuffers"]
constant glDeleteBuffers : List GLBufferObject → IO Unit

inductive BufferTarget
| ArrayBuffer
| ElementBuffer
| TextureBuffer
| UniformBuffer

@[extern "lean_convert_gl_buffer_target"]
constant convertGLBufferTarget : BufferTarget → UInt32

@[extern "lean_opengl_glbindbuffer"]
constant glBindBuffer : BufferTarget → GLBufferObject → IO Unit

inductive BufferFrequency
| StreamBuffer   -- written once and used a few times
| StaticBuffer   -- written once, used many times
| DynamicBuffer  -- written many times, used many times
  
inductive BufferAccessPattern
| DrawBufferAccess
| ReadBufferAccess
| CopyBufferAccess

@[extern "lean_opengl_glbufferdata_bytes"]
constant glBufferDataBytes : BufferTarget → ByteArray → BufferFrequency → BufferAccessPattern → IO Unit

@[extern "lean_opengl_glbufferdata_floats"]
constant glBufferDataFloats : BufferTarget → FloatArray → BufferFrequency → BufferAccessPattern → IO Unit



inductive ShaderType
| ComputeShader
| VertexShader
| TessellationControlShader
| TessellationEvaluationShader
| GeometryShader
| FragmentShader

def GLShaderObject := UInt64

instance : ToString GLShaderObject where
  toString x := let (y : UInt64) := x
                toString y

@[extern "lean_opengl_glcreateshader"]
constant glCreateShader : ShaderType → IO GLShaderObject

@[extern "lean_openl_glshadersource"]
constant glShaderSource : GLShaderObject → List String → IO Unit

constant glCompileShader : GLShaderObject → IO Unit

end OpenGL
