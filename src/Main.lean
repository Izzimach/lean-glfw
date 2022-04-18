import GLFW
import GLFW.OpenGL


open GLFW
open OpenGL
open BufferStorageFlags

def glfwWrap (wrapped : Window → IO Unit) : ExceptT String IO Unit := do
    glfwInit
    IO.println "Glfw init success!"
    try do
        let w <- glfwCreateWindow
        try do
            glfwMakeContextCurrent w
            wrapped w
        finally
            glfwDestroyWindow w
    finally
        glfwTerminate

partial def renderLoop : Int → Window → GLVertexArrayObject → GLProgramObject → IO Unit :=
  fun c w vao prog => do
    let bits := glClearBits [ClearBufferEnum.ClearColorBuffer, ClearBufferEnum.ClearDepthBuffer]
    glClear bits

    let mvMatrix := FloatArray.mk $ Array.mk [1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1]
    let matLocation <- glGetUniformLocation prog "modelViewMatrix"
    glProgramUniformMatrix4fv prog matLocation mvMatrix

    let projLocation <- glGetUniformLocation prog "projectionMatrix"
    let pjMatrix := FloatArray.mk $ Array.mk [1,0,0,0, 0,1,0,0, 0,0,-1,0, 0,0,0,1]
    glProgramUniformMatrix4fv prog projLocation pjMatrix

    glBindVertexArray vao
    glUseProgram prog
    glDrawArrays GLDrawMode.GLTriangles 0 3

    glfwSwapBuffers w
    glfwPollEvents
    let terminate <- glfwWindowShouldClose w
    if (terminate || c < 0)
    then return ()
    else renderLoop (c-1) w vao prog

def vertexData := FloatArray.mk <| Array.mk [-0.5,-0.7,0.0, 0.5,-0.7,0.0, 0.0,0.68,0.0, 1,0,0]

def textureBytes := ByteArray.mk <| Array.mk [0,255,255,0, 0,255,0,0, 0,0,255,0, 0,255,255,0]

def vertexShader := [
"#version 450 core
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
layout (location=0) in vec3 position;
out vec2 texCoordFrag;
"
,
"const vec4 positions[] = vec4[]( vec4(-0.5f, -0.7f,    0.0, 1.0), 
                                 vec4( 0.5f, -0.7f,    0.0, 1.0),    
                                 vec4( 0.0f,  0.6888f, 0.0, 1.0));"
,
"void main() {
  //gl_Position = vec4(position, 1);
  gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1);
  //gl_Position = positions[gl_VertexID];
  texCoordFrag = gl_Position.xy;
}"
]

def fragmentShader := [
"#version 450

uniform sampler2D tex;
in vec2 texCoordFrag;
out vec4 fragColor;
",
"
void main() {
  fragColor = texture (tex, texCoordFrag*3);// + vec4(0.1,0.1,0.1,0);
              //vec4(0,1,1,0);
}"
]

def buildShader : ShaderType → List String → IO GLShaderObject := fun sType source => do
    let vshaderID <- glCreateShader sType
    IO.println ("shader ID: " ++ toString vshaderID)
    glShaderSource vshaderID source
    glCompileShader vshaderID
    return vshaderID

def startRender : Window → IO Unit :=
  fun w => do
    enableGLDebugOutput
    glfwSwapInterval 1
    let ⟨width,height⟩ <- glfwGetFramebufferSize w
    glViewport 0 0 width height
    glClearColor 1.0 0.0 0.0 0.0
    let buffers <- OpenGL.glCreateBuffers 1
    match (buffers.get? 0) with
    | Option.none => return ()
    | Option.some vBuf => do
        glNamedBufferStorage_Floats vBuf vertexData [GLMapWrite, GLDynamicStorage]

        let fshaderID <- buildShader ShaderType.VertexShader vertexShader
        let vshaderID <- buildShader ShaderType.FragmentShader fragmentShader
        let programID <- glCreateProgram
        glAttachShader programID fshaderID
        glAttachShader programID vshaderID
        glLinkProgram programID
        --glUseProgram programID

        let vaos <- glCreateVertexArrays 1
        IO.println ("vaos=" ++ toString vaos)
        match (vaos.get? 0) with
        | Option.none => return ()
        | Option.some vao => do
            glVertexArrayAttribBinding vao 0 0
            glVertexArrayVertexBuffer vao 0 vBuf 0 12
            glEnableVertexArrayAttrib vao 0
            glVertexArrayAttribFormat vao 0 3 GLDataType.GLFloat false 0

            let textures <- glCreateTextures GLTextureTarget.GLTexture2D 2
            IO.println ("textures=" ++ toString textures)
            match (textures.get? 0) with
            | Option.none => return ()
            | Option.some tObj => do
                glTextureStorage2D tObj 1 GLSizedTextureFormat.R8 4 4
                glTextureSubImage2D tObj 0 0 0 4 4 GLPixelFormat.Red GLPixelType.UByte textureBytes
                glBindTextureUnit 0 tObj

                renderLoop 100 w vao programID

            glDeleteVertexArrays vaos
            glDeleteProgram programID
            glDeleteShader fshaderID
            glDeleteShader vshaderID
            glDeleteBuffers buffers

    
def main : IO Unit := do
    let r <- ExceptT.run $ glfwWrap startRender
    match r with
    | Except.ok a => IO.println ("Ok: " ++ toString a)
    | Except.error e => IO.println ("An error occurred! " ++ e)
