
namespace GLFW

-- I don't know what this does
constant WindowT : NonemptyType
def Window := WindowT.type

@[extern "lean_glfw_init"]
constant glfwInit : IO Unit

@[extern "lean_glfw_terminate"]
constant glfwTerminate : IO Unit

@[extern "lean_glfw_create_window"]
constant glfwCreateWindow : IO Window

-- this makes the context and also calls the glad API loader
@[extern "lean_glfw_make_context_current"]
constant glfwMakeContextCurrent : @& Window → IO Unit

@[extern "lean_glfw_destroy_window"]
constant glfwDestroyWindow : Window → IO Unit


@[extern "lean_glfw_get_framebuffer_size"]
constant glfwGetFramebufferSize : @& Window → IO (Int × Int)

@[extern "lean_glfw_window_should_close"]
constant glfwWindowShouldClose : @& Window → IO Bool

@[extern "lean_glfw_get_time"]
constant glfwGetTime : IO Double

@[extern "lean_glfw_swap_buffers"]
constant glfwSwapBuffers : Window → IO Unit

@[extern "lean_glfw_swap_interval"]
constant glfwSwapInterval : Int → IO Unit

@[extern "lean_glfw_poll_events"]
constant glfwPollEvents : IO Unit

end GLFW