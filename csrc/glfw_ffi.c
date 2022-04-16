
#include <lean/lean.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "data_marshal.h"

#include <stdio.h>

// glfwInit : IO Unit
//

lean_obj_res lean_glfw_init()
{
    if (glfwInit()) {
        return lean_return_unit();
    }
    else {
        return lean_io_result_mk_error(lean_mk_io_user_error(lean_mk_string("Error calling glfwInit")));
    }
}

// glfwCreateWindow : IO Window
//
lean_obj_res lean_glfw_create_window()
{
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    GLFWwindow* window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (window == NULL) {
        return lean_io_result_mk_error(lean_mk_io_user_error(lean_mk_string("Error calling glfwCreateWindow")));
    }
    return lean_io_result_mk_ok(lean_mk_ptr(window));
}

// glfwMakeCurrentContext : Window -> IO Unit
lean_obj_res lean_glfw_make_context_current(b_lean_obj_arg lw)
{
   GLFWwindow *window = (GLFWwindow *)(lean_get_external_data(lw));
   if (window == NULL) {
        return lean_io_result_mk_error(lean_mk_io_user_error(lean_mk_string("NULL window pointer passed to glfwMakeCurrentContext")));
   }
   else
   {
        glfwMakeContextCurrent(window);
       
        if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
            return lean_io_result_mk_error(lean_mk_io_user_error(lean_mk_string("Call to 'gladLoadGLLoader' failed in glfwMakeCurrentContext")));
        }
        else {
            return lean_return_unit();
        }
   }
}

// glfwDestroyWindow : Window -> IO Unit
//
lean_obj_res lean_glfw_destroy_window(lean_obj_arg lw)
{
   GLFWwindow *window = (GLFWwindow *)(lean_get_external_data(lw));
   if (window == NULL) {
        return lean_io_result_mk_error(lean_mk_io_user_error(lean_mk_string("NULL window pointer passed to glfwDestroyWindow")));
   }
   else {
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

// glfwGetFrameBufferSize : Window -> IO (Int * Int)
//
lean_obj_res lean_glfw_get_framebuffer_size(lean_obj_arg lw)
{
   GLFWwindow *window = (GLFWwindow *)(lean_get_external_data(lw));
   if (window == NULL) {
        return lean_io_result_mk_error(lean_mk_io_user_error(lean_mk_string("NULL window pointer passed to glfwGetFrameBufferSize")));
   }
   else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        lean_object *a = lean_box(width);
        lean_object *b = lean_box(height);
        lean_object* tuple = lean_alloc_ctor(0, 2, 0);
        lean_ctor_set(tuple, 0, a);
        lean_ctor_set(tuple, 1, b);
        return lean_io_result_mk_ok(tuple);
   }
}

// glfwWindowShouldClose : Window -> IO Bool
//
lean_obj_res lean_glfw_window_should_close(lean_obj_arg lw)
{
   GLFWwindow *window = (GLFWwindow *)(lean_get_external_data(lw));
   if (window == NULL) {
        return lean_io_result_mk_error(lean_mk_io_user_error(lean_mk_string("NULL window pointer passed to glfwWindowShouldClose")));
   }
   else {
       bool result = glfwWindowShouldClose(window);
       return lean_io_result_mk_ok(lean_box(result));
   }
}

// glfwGetTime : IO Double
//
lean_obj_res lean_glfw_get_time()
{
    return lean_io_result_mk_ok(lean_box_float(glfwGetTime()));
}

// glfwSwapBuffers : Window -> IO Unit
//
lean_obj_res lean_glfw_swap_buffers(lean_obj_arg lw)
{
    GLFWwindow *window = (GLFWwindow *)(lean_get_external_data(lw));
    if (window == NULL) {
        return lean_io_result_mk_error(lean_mk_io_user_error(lean_mk_string("NULL window pointer passed to glfwSwapBuffers")));
    }
    else {
        glfwSwapBuffers(window);
    }
    return lean_return_unit();
}

// constant glfwSwapInterval : Int → IO Unit
//
lean_obj_res lean_glfw_swap_interval(lean_obj_arg l)
{
    int interval = lean_scalar_to_int(l);
    glfwSwapInterval(interval);
    return lean_return_unit();
}

// glfwPollEvents : IO Unit
//
lean_obj_res lean_glfw_poll_events()
{
    glfwPollEvents();
    return lean_return_unit();
}