
#include <lean/lean.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "data_marshal.h"

#include <stdio.h>

// glfwInit : IO Bool
//

lean_obj_res lean_glfw_init()
{
    if (glfwInit()) {
        return lean_io_result_mk_ok(lean_box(lean_uint32_to_uint8(1)));
    }
    else {
        return lean_io_result_mk_ok(lean_box(lean_uint32_to_uint8(0)));
    }
}

// glfwCreateWindow : IO (Option Window)/
//
lean_obj_res lean_glfw_create_window()
{
    GLFWwindow* window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (window == NULL)
    {
        return lean_io_result_mk_ok(lean_mk_option_none());
    }
    return lean_io_result_mk_ok(lean_mk_packptr(window));
}

// glfwMakeCurrentContext : Window -> IO Unit
lean_obj_res lean_glfw_make_context_current(lean_obj_arg lw)
{
   GLFWwindow *window = (GLFWwindow *)(lean_get_external_data(lw));
   if (window == NULL)
   {
     // error?
   }
   else
   {
        printf ("binding OpenGL\n");
        glfwMakeContextCurrent(window);
       
        if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
            printf("Failed to initialize OpenGL context");
            return lean_return_unit();
        }
        printf("glad loader completed!\n");
   }
   return lean_return_unit();
}

// glfwDestroyWindow : Window -> IO Unit
//
lean_obj_res lean_glfw_destroy_window(lean_obj_arg lw)
{
   GLFWwindow *window = (GLFWwindow *)(lean_get_external_data(lw));
   if (window == NULL)
   {
     // error?
   }
   else
   {
       printf ("destroying!\n");
       glfwDestroyWindow(window);
   }
   return lean_return_unit();
}

// glfwTerminate : IO Unit
//
lean_obj_res lean_glfw_terminate()
{
    glfwTerminate();
    return lean_return_unit();
}

