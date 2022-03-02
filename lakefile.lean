import Lake
open System Lake Lean DSL

-- edit these to point to your directories for GLFW and the windows SDK
constant glfwDir : FilePath := "d:\\Development\\glfw-3.3.6"
def winSDKDirectory : FilePath := "c:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.18362.0\\um\\x64"



def buildDir := defaultBuildDir
def cDir : FilePath := "csrc"
--def ffiSrc := cDir / "glfw_bindings.c"

def glfwIncludeDir := glfwDir / "include"
def gladLocalDir := "glad"
def glfwLibDir := glfwDir / "lib-mingw-w64"
def gdiFile := winSDKDirectory / "Gdi32.lib"

-- given a package dir and c source file, constructs a FileTarget that builds the
-- corresponding .o file using the lean C compiler
def ffiOTarget (pkgDir : FilePath) (ffiSrc : FilePath): FileTarget :=
  let oFile := pkgDir / buildDir / cDir / (System.FilePath.withExtension ffiSrc "o")
  let srcTarget := inputFileTarget <| pkgDir / cDir/ ffiSrc
  let localIncludeDir := pkgDir / cDir / "include"
  fileTargetWithDep oFile srcTarget fun srcFile => do
    let compileOptions := #[
                              "-I", (← getLeanIncludeDir).toString,
                              "-I", glfwIncludeDir.toString,
                              "-I", localIncludeDir.toString
    ]
    compileO oFile srcFile compileOptions "cc"

-- a FileTarget to build the ffi lib from component c files
def cLibTarget (pkgDir : FilePath) : FileTarget :=
  let libFile := pkgDir / buildDir / cDir / "libffi.a"
  staticLibTarget libFile #[
                            ffiOTarget pkgDir "glfw_ffi.c",
                            ffiOTarget pkgDir "data_marshal.c",
                            ffiOTarget pkgDir "glad.c"
                            ]


package GLFW (pkgDir) (args) {
  srcDir := "src"


  -- specify the c binding library as an additional target
  moreLibTargets := #[cLibTarget pkgDir]

  -- The final executable needs to link in glfw and gdi32 as well as the cLibTarget
  moreLinkArgs := #["-L", glfwLibDir.toString, "-lglfw3", gdiFile.toString]

}
