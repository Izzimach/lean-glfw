import GLFW

open GLFW

def main : IO Unit := do
  let initResult ← glfwInit
  if initResult
  then do
    IO.println "Glfw init success!"
    try do
      let w0 <- glfwCreateWindow
      match w0 with
      | Option.none => pure ()
      | Option.some w => do
          glfwMakeContextCurrent w
          glfwDestroyWindow w
    finally
      glfwTerminate
  else IO.println "init failed!"

