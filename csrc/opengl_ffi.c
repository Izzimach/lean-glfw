#include <lean/lean.h>

#include <glad/glad.h>

#include "data_marshal.h"

#include <stdio.h>

//constant glViewport : Int → Int → Int → Int → IO Unit
//
lean_obj_res lean_opengl_glviewport(lean_obj_arg lx, lean_obj_arg ly, lean_obj_arg lwidth, lean_obj_arg lheight)
{
    int x = lean_scalar_to_int(lx);
    int y = lean_scalar_to_int(ly);
    int width = lean_scalar_to_int(lwidth);
    int height = lean_scalar_to_int(lheight);
    glViewport(x,y,width,height);
    return lean_return_unit();
}

// constant glClearBits : List GLClearBufferEnum → UInt32
uint32_t lean_opengl_glclearbits(lean_obj_arg enum_list)
{
    uint32_t clearbits = 0;
    lean_object *element = enum_list;
    while (!lean_is_scalar(element)) {
        lean_object * head = lean_ctor_get(element, 0);
        unsigned clearbit = lean_obj_tag(head);
        switch (clearbit) {
            case 0: // ClearColorBuffer
                clearbits |= GL_COLOR_BUFFER_BIT;
                break;
            case 1: // ClearDepthBuffer
                clearbits |= GL_DEPTH_BUFFER_BIT;
                break;
            case 2: // ClearStencilBuffer
                clearbits |= GL_STENCIL_BUFFER_BIT;
                break;
            default: // UNKNOWN
                break;
        }
        element = lean_ctor_get(element, 1);
    }
    return clearbits;
}

//constant glClear : UInt32 →  IO Unit
//
lean_obj_res lean_opengl_glclear (uint32_t bits)
{
    glClear(bits);
    return lean_return_unit();
}

//constant glClearColor : Float → Float → Float → Float → IO Unit
//
lean_obj_res lean_opengl_glclearcolor(double red, double green, double blue, double alpha)
{
    glClearColor(red,green,blue,alpha);
    return lean_return_unit();
}