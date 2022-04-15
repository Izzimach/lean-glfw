
namespace OpenGL

--
-- hook up debug callback
--
@[extern "lean_opengl_debugoutput"]
constant enableGLDebugOutput : IO Unit

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
constant glClear : (bits : UInt64) →  IO Unit

@[extern "lean_opengl_glclearcolor"]
constant glClearColor : Float → Float → Float → Float → IO Unit

--
-- Buffers
--

def GLBufferObject := UInt32

instance : ToString GLBufferObject where
    toString x := let (y : UInt32) := x; "GLBuffer: " ++ toString y

@[extern "lean_opengl_glgenbuffers"]
constant glGenBuffers : (count : UInt32) → IO (List GLBufferObject)

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

-- note that lean "FloatArray" is an array of C doubles
@[extern "lean_opengl_glbufferdata_doubles"]
constant glBufferDataDoubles : BufferTarget → FloatArray → BufferFrequency → BufferAccessPattern → IO Unit

-- since FloatArray elements are C doubles this will truncate elements down to C floats
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
  toString x := let (y : UInt64) := x; "GLShader: " ++ toString y

@[extern "lean_opengl_glcreateshader"]
constant glCreateShader : ShaderType → IO GLShaderObject

@[extern "lean_openl_gldeleteshader"]
constant glDeleteShader : GLShaderObject → IO Unit

@[extern "lean_openl_glshadersource"]
constant glShaderSource : GLShaderObject → List String → IO Unit

@[extern "lean_opengl_glcompileshader"]
constant glCompileShader : GLShaderObject → IO Unit

def GLProgramObject := UInt64

instance : ToString GLProgramObject where
  toString x := let (y : UInt64) := x; "GLProgram: " ++ toString y

@[extern "lean_opengl_createprogram"]
constant glCreateProgram : IO GLProgramObject

@[extern "lean_openl_deleteprogram"]
constant glDeleteProgram : GLProgramObject → IO Unit

@[extern "lean_opengl_attachshader"]
constant glAttachShader : GLProgramObject → GLShaderObject → IO Unit

@[extern "lean_opengl_linkprogram"]
constant glLinkProgram : GLProgramObject → IO Unit

@[extern "lean_opengl_useprogram"]
constant glUseProgram : GLProgramObject → IO Unit

@[extern "lean_opengl_getuniformlocation"]
constant glGetUniformLocation : GLProgramObject → String → IO UInt32

@[extern "lean_opengl_programuniformmatrix4fv"]
constant glProgramUniformMatrix4fv : GLProgramObject → (location : UInt32) → FloatArray → IO Unit




def GLVertexArrayObject := UInt64

instance : ToString GLVertexArrayObject where
  toString x := let (y : UInt64) := x; "GLVertexArrayObject: " ++ toString y

@[extern "lean_opengl_genvertexarrays"]
constant glGenVertexArrays : UInt32 → IO (Array GLVertexArrayObject)

@[extern "lean_opengl_bindvertexarray"]
constant glBindVertexArray : GLVertexArrayObject → IO Unit

@[extern "lean_opengl_deletevertexarrays"]
constant glDeleteVertexArrays : Array GLVertexArrayObject → IO Unit



@[extern "lean_opengl_glbindvertexbuffer"]
constant glBindVertexBuffer : (bindingIndex : UInt64) → GLBufferObject → (offset : UInt64) → (stride : UInt64) → IO Unit

@[extern "lean_opengl_enablevertexattribarray"]
constant glEnableVertexAttribArray : (attribIndex : UInt64) → IO Unit

@[extern "lean_opengl_disablevertexattribarray"]
constant glDisableVertexAttribArray : (attribIndex : UInt64) → IO Unit

inductive GLDataType where
| GLByte
| GLUnsignedByte
| GLShort
| GLUnsignedShort
| GLInt
| GLUnsignedInt
| GLFloat
| GLDouble

@[extern "lean_opengl_vertexattribformat"]
constant glVertexAttribFormat : (attribIndex : UInt64) → (size : UInt64) → GLDataType → (normalized : Bool) → (relativeOffset : UInt64) → IO Unit

@[extern "lean_opengl_vertexattribbinding"]
constant glVertexAttribBinding : (attribIndex : UInt64) → (bindingIndex : UInt64) → IO Unit

inductive GLDrawMode where
| GLPoints
| GLLines
| GLTriangleStrip
| GLTriangleFan
| GLTriangles

@[extern "lean_opengl_drawarrays"]
constant glDrawArrays : GLDrawMode → (first : UInt64) → (count : UInt64) → IO Unit


end OpenGL
