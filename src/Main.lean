import GLFW
import GLFW.OpenGL

open GLFW

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
    let bits := OpenGL.glClearBits [OpenGL.GLClearBufferEnum.ClearColorBuffer, OpenGL.GLClearBufferEnum.ClearDepthBuffer]
    OpenGL.glClear bits
    glfwSwapBuffers w
    glfwPollEvents
    let terminate <- glfwWindowShouldClose w
    if (terminate || c < 0)
    then return ()
    else renderLoop (c-1) w

def startRender : Window → IO Unit :=
  fun w => do
    glfwSwapInterval 1
    let ⟨width,height⟩ <- glfwGetFramebufferSize w
    OpenGL.glViewport 0 0 width height
    OpenGL.glClearColor 1.0 0.0 0.0 0.0
    renderLoop 200 w
    
def main : IO Unit := do
    let r <- ExceptT.run $ glfwWrap startRender
    match r with
    | Except.ok a => IO.println ("Ok: " ++ toString a)
    | Except.error e => IO.println ("An error occurred! " ++ e)

