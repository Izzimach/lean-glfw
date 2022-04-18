
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

@[extern "lean_opengl_glcreatebuffers"]
constant glCreateBuffers : (count : UInt32) → IO (List GLBufferObject)

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

inductive BufferStorageFlags
  | GLDynamicStorage
  | GLMapRead
  | GLMapWrite
  | GLMapPersistent
  | GLMapCoherent
  | GLClientStorage

-- glNamedBufferStorage specifically for a FloatArray, put into buffer as GLfloat
@[extern "lean_opengl_glnamedbufferstorage_floats"]
constant glNamedBufferStorage_Floats : GLBufferObject → FloatArray → List BufferStorageFlags → IO Unit

-- glNamedBufferStorage specifically for a FloatArray, stored as GLdouble
@[extern "lean_opengl_glnamedbufferstorage_doubles"]
constant glNamedBufferStorage_Doubles : GLBufferObject → FloatArray → List BufferStorageFlags → IO Unit

@[extern "lean_opengl_glNamedBufferStorage_uint32"]
constant glNamedBufferStorage_UInt32 : GLBufferObject → Array UInt32 → List BufferStorageFlags → IO Unit

@[extern "lean_opengl_glnamedbufferstorage_bytes"]
constant glNamedBufferStorage_Bytes : GLBufferObject → ByteArray → List BufferStorageFlags → IO Unit

inductive ShaderType
  | ComputeShader
  | VertexShader
  | TessellationControlShader
  | TessellationEvaluationShader
  | GeometryShader
  | FragmentShader

def GLShaderObject := UInt32

instance : ToString GLShaderObject where
  toString x := let (y : UInt32) := x; "GLShader: " ++ toString y

@[extern "lean_opengl_glcreateshader"]
constant glCreateShader : ShaderType → IO GLShaderObject

@[extern "lean_openl_gldeleteshader"]
constant glDeleteShader : GLShaderObject → IO Unit

@[extern "lean_openl_glshadersource"]
constant glShaderSource : GLShaderObject → List String → IO Unit

@[extern "lean_opengl_glcompileshader"]
constant glCompileShader : GLShaderObject → IO Unit

def GLProgramObject := UInt32

instance : ToString GLProgramObject where
  toString x := let (y : UInt32) := x; "GLProgram: " ++ toString y

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




def GLVertexArrayObject := UInt32

instance : ToString GLVertexArrayObject where
  toString x := let (y : UInt32) := x; "GLVertexArrayObject: " ++ toString y

@[extern "lean_opengl_genvertexarrays"]
constant glGenVertexArrays : (count : UInt32) → IO (Array GLVertexArrayObject)

@[extern "lean_opengl_createvertexarrays"]
constant glCreateVertexArrays : (count : UInt32) → IO (Array GLVertexArrayObject)

@[extern "lean_opengl_bindvertexarray"]
constant glBindVertexArray : GLVertexArrayObject → IO Unit

@[extern "lean_opengl_deletevertexarrays"]
constant glDeleteVertexArrays : Array GLVertexArrayObject → IO Unit



@[extern "lean_opengl_glbindvertexbuffer"]
constant glBindVertexBuffer : (bindingIndex : UInt32) → GLBufferObject → (offset : UInt64) → (stride : UInt64) → IO Unit

@[extern "lean_opengl_glvertexarrayvertexbuffer"]
constant glVertexArrayVertexBuffer : GLVertexArrayObject → (bindingIndex : UInt32) → GLBufferObject → (offset : UInt64) → (stride : UInt64) → IO Unit

@[extern "lean_opengl_enablevertexattribarray"]
constant glEnableVertexAttribArray : (attribIndex : UInt32) → IO Unit

@[extern "lean_opengl_disablevertexattribarray"]
constant glDisableVertexAttribArray : (attribIndex : UInt32) → IO Unit

@[extern "lean_opengl_enablevertexarrayattrib"]
constant glEnableVertexArrayAttrib : GLVertexArrayObject → (attribIndex : UInt32) → IO Unit

@[extern "lean_opengl_disablevertexarrayattrib"]
constant glDisableVertexArrayAttrib : GLVertexArrayObject → (attribIndex : UInt32) → IO Unit

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
constant glVertexAttribFormat : (attribIndex : UInt32) → (size : UInt64) → GLDataType → (normalized : Bool) → (relativeOffset : UInt64) → IO Unit

@[extern "lean_opengl_vertexarrayattribformat"]
constant glVertexArrayAttribFormat : GLVertexArrayObject → (attribIndex : UInt32) → (size : UInt64) → GLDataType → (normalized : Bool) → (relativeOffset : UInt64) → IO Unit

@[extern "lean_opengl_vertexattribbinding"]
constant glVertexAttribBinding : (attribIndex : UInt32) → (bindingIndex : UInt32) → IO Unit

@[extern "lean_opengl_vertexarrayattribbinding"]
constant glVertexArrayAttribBinding : GLVertexArrayObject → (attribIndex : UInt32) → (bindingIndex : UInt32) → IO Unit

inductive GLDrawMode where
  | GLPoints
  | GLLines
  | GLTriangleStrip
  | GLTriangleFan
  | GLTriangles

@[extern "lean_opengl_drawarrays"]
constant glDrawArrays : GLDrawMode → (first : UInt64) → (count : UInt64) → IO Unit

def GLTextureObject := UInt32

instance : ToString GLTextureObject where
    toString x := let (y : UInt32) := x; "GLTexture: " ++ toString y

inductive GLTextureTarget
  | GLTexture1D
  | GLTexture2D
  | GLTexture3D

@[extern "lean_opengl_createtextures"]
constant glCreateTextures : GLTextureTarget → (count : UInt32) → IO (Array GLTextureObject)

@[extern "lean_opengl_deletetextures"]
constant glDeleteTextures : Array GLTextureObject → IO Unit

inductive GLSizedTextureFormat
  | R8
  | RG8
  | RGB8
  | RGBA8
  | R16F
  | R32F

@[extern "lean_opengl_texturestorage2d"]
constant glTextureStorage2D : GLTextureObject → (levels : UInt32) → GLSizedTextureFormat → (width : UInt32) → (height : UInt32) → IO Unit


inductive GLPixelFormat
| Red
| RG
| RGB
| BGR
| RGBA
| BGRA
| Depth
| Stencil

inductive GLPixelType
| UByte
| Byte
| UShort
| Short
| UInt
| Int
| Float

@[extern "lean_opengl_texturesubimage2d"]
constant glTextureSubImage2D : GLTextureObject → (level : UInt32) → (xoffset : Int32) → (yoffset : Int32) → (width : UInt32) → (height :  UInt32) → GLPixelFormat → GLPixelType → ByteArray → IO Unit

@[extern "lean_opengl_bindtextureunit"]
constant glBindTextureUnit : (unit : UInt32) → GLTextureObject → IO Unit

end OpenGL
