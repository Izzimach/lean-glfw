import Lake
open System Lake Lean DSL

-- edit these to point to your directories for GLFW and the windows SDK
constant glfwDir : FilePath := "d:\\Development\\glfw-3.3.6"
def winSDKDirectory : FilePath := "c:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.18362.0\\um\\x64"



def buildDir := defaultBuildDir
def cDir : FilePath := "csrc"
def ffiSrc := cDir / "glfw_bindings.c"

def glfwIncludeDir := glfwDir / "include"
def glfwLibDir := glfwDir / "lib-mingw-w64"
def gdiFile := winSDKDirectory / "Gdi32.lib"

def ffiOTarget (pkgDir : FilePath) : FileTarget :=
  let oFile := pkgDir / buildDir / cDir / "ffi.o"
  let srcTarget := inputFileTarget <| pkgDir / ffiSrc
  fileTargetWithDep oFile srcTarget fun srcFile => do
    compileO oFile srcFile #["-I", (← getLeanIncludeDir).toString, "-I", glfwIncludeDir.toString] "cc"

def cLibTarget (pkgDir : FilePath) : FileTarget :=
  let libFile := pkgDir / buildDir / cDir / "libffi.a"
  staticLibTarget libFile #[ffiOTarget pkgDir]

package «lean-glfw» (pkgDir) (args) {
  srcDir := "src"

  --libRoots := #[`Ffi]

  -- specify the c bindings as an additional target
  moreLibTargets := #[cLibTarget pkgDir]

  -- The final executable needs to link in glfw and gdi32
  moreLinkArgs := #["-L", glfwLibDir.toString, "-lglfw3", gdiFile.toString]

}
