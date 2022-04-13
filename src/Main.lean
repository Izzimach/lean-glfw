import GLFW
import GLFW.OpenGL

open GLFW
open OpenGL

def glfwWrap (wrapped : Window → IO Unit) : IO Unit  := do
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
    glfwSwapBuffers w
    glfwPollEvents
    let terminate <- glfwWindowShouldClose w
    if (terminate || c < 0)
    then return ()
    else renderLoop (c-1) w

def vertexData := FloatArray.mk $ Array.mk [0,0,0, 0,1,0, 1,1,0, 1,0,0]

def someShader := [
"#version 330
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
in vec3 position;",
"void main() {
  gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1);
  //gl_Position = vec4(pos2,0,1);
  //texCoordFrag = texCoord;
}"
]

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
    let shaderID <- glCreateShader ShaderType.VertexShader
    IO.println ("shader ID: " ++ toString shaderID)
    glShaderSource shaderID someShader
    renderLoop 200 w
    glDeleteBuffers buffers
    
def main : IO Unit := do
    let r <- ExceptT.run $ glfwWrap startRender
    match r with
    | Except.ok a => IO.println ("Ok: " ++ toString a)
    | Except.error e => IO.println ("An error occurred! " ++ e)

