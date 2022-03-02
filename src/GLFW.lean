
namespace GLFW

-- I don't know what this does
constant WindowT : NonemptyType
def Window := WindowT.type

@[extern "lean_glfw_init"] constant glfwInit : IO Bool
@[extern "lean_glfw_terminate"] constant glfwTerminate : IO Unit
@[extern "lean_glfw_create_window"] constant glfwCreateWindow : IO (Option Window)
@[extern "lean_glfw_make_context_current"] constant glfwMakeContextCurrent : Window → IO Unit
@[extern "lean_glfw_destroy_window"] constant glfwDestroyWindow : Window → IO Unit

end GLFW