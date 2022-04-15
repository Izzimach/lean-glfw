import GLFW
import GLFW.OpenGL

open GLFW
open OpenGL

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

partial def renderLoop : Int → Window → IO Unit :=
  fun c w => do
    let bits := glClearBits [ClearBufferEnum.ClearColorBuffer, ClearBufferEnum.ClearDepthBuffer]
    glClear bits
    glDrawArrays GLDrawMode.GLTriangles 0 3
    glfwSwapBuffers w
    glfwPollEvents
    let terminate <- glfwWindowShouldClose w
    if (terminate || c < 0)
    then return ()
    else renderLoop (c-1) w

def vertexData := FloatArray.mk $ Array.mk [-0.5,-0.7,0.0, 0.5,-0.7,0.0, 0.0,0.68,0.0, 1,0,0]

def vertexShader := [
"#version 330
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
in vec3 position;
"
,
"const vec4 positions[] = vec4[]( vec4(-0.5f, -0.7f,    0.0, 1.0), 
                                 vec4( 0.5f, -0.7f,    0.0, 1.0),    
                                 vec4( 0.0f,  0.6888f, 0.0, 1.0));"
,
"void main() {
  gl_Position = positions[gl_VertexID];
}"
]

def fragmentShader := [
"#version 330

//uniform sampler2D tex;
//in vec2 texCoordFrag;
out vec4 fragColor;
",
"
void main() {
  fragColor = vec4(0,1,1,0);
}"
]

def buildShader : ShaderType → List String → IO GLShaderObject := fun sType source => do
    let vshaderID <- glCreateShader sType
    IO.println ("vertex shader ID: " ++ toString vshaderID)
    glShaderSource vshaderID source
    glCompileShader vshaderID
    return vshaderID

def startRender : Window → IO Unit :=
  fun w => do
    glfwSwapInterval 1
    let ⟨width,height⟩ <- glfwGetFramebufferSize w
    glViewport 0 0 width height
    glClearColor 1.0 0.0 0.0 0.0
    let buffers <- OpenGL.glGenBuffers 2
    IO.println ("buffers: " ++ toString buffers)
    match (buffers.get? 0) with
    | Option.none => return ()
    | Option.some b => do
        glBindBuffer BufferTarget.ArrayBuffer b
        glBufferDataFloats BufferTarget.ArrayBuffer vertexData BufferFrequency.StaticBuffer BufferAccessPattern.DrawBufferAccess

        let fshaderID <- buildShader ShaderType.VertexShader vertexShader
        let vshaderID <- buildShader ShaderType.FragmentShader fragmentShader
        let programID <- glCreateProgram
        glAttachShader programID fshaderID
        glAttachShader programID vshaderID
        glLinkProgram programID
        glUseProgram programID

        let vaos <- glGenVertexArrays 1
        match (vaos.get? 0) with
        | Option.none => return ()
        | Option.some vao => do
            glBindVertexArray vao
            
            glEnableVertexAttribArray 0
            glVertexAttribFormat 0 3 GLDataType.GLFloat false 0
            glVertexAttribBinding 0 0
            glBindVertexBuffer 0 b 0 12

            --let matLocation <- glGetUniformLocation programID "modelViewMatrix"
            --IO.println ("modelView location: " ++ toString matLocation)
            --let projLocation <- glGetUniformLocation programID "projectionMatrix"
            --IO.println ("projectionlocation: " ++ toString projLocation)

            --let mvMatrix := FloatArray.mk $ Array.mk [1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1]
            --let pjMatrix := FloatArray.mk $ Array.mk [1,0,0,0, 0,1,0,0, 0,0,-1,0, 0,0,0,1]
            --glProgramUniformMatrix4fv programID matLocation mvMatrix
            --glProgramUniformMatrix4fv programID projLocation pjMatrix

            renderLoop 100 w

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
