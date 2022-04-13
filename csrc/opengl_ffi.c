#include <lean/lean.h>

#include <glad/glad.h>

#include "data_marshal.h"

#include <stdio.h>
#include <stdlib.h>

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

// constant glClearBits : List GLClearBufferEnum → UInt64
//
uint64_t lean_opengl_glclearbits(lean_obj_arg enum_list)
{
    uint64_t clearbits = 0;

    // walk the list and OR in each bit
    lean_object *current_element = enum_list;
    while (!lean_is_scalar(current_element)) {
        // element is a "cons head tail" object
        lean_object * head = lean_ctor_get(current_element, 0);
        lean_object * tail = lean_ctor_get(current_element, 1);

        unsigned currentbit = lean_obj_tag(head);
        // the tag value is the index into the inductive "ClearBufferEnum" datatype
        switch (currentbit) {
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
        // advance to next element
        current_element = tail;
    }
    return clearbits;
}

//constant glClear : UInt64 →  IO Unit
//
lean_obj_res lean_opengl_glclear (uint64_t bits)
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

//
// 

//constant glGenBuffers : UInt32 → IO (List GLBufferObject))
//
lean_obj_res lean_opengl_glgenbuffers(uint32_t bufferCount)
{
    GLuint *bufferData = calloc(bufferCount, sizeof(GLuint));
    glGenBuffers(bufferCount, bufferData);

    // build a Lean list from the results
    lean_object *resultList = lean_box(0); // List.nil
    for (int bix=0; bix < bufferCount; bix++)
    {
      // build a cons node and prepend to the current resultList
      lean_object *node = lean_alloc_ctor(1,2,0); // List.cons
      lean_ctor_set(node, 0, lean_box(bufferData[bix])); // head
      lean_ctor_set(node, 1, resultList);                // tail
      resultList = node;
    }

    free(bufferData);
    return lean_io_result_mk_ok(resultList);
}


// constant glDeleteBuffers : List GLBufferObject → IO Unit
//
lean_obj_res lean_opengl_gldeletebuffers(lean_obj_arg bufferList)
{
    // walk the list
    lean_object *current_element = bufferList;
    while (!lean_is_scalar(current_element)) {
        // element is a "cons head tail" object
        lean_object * head = lean_ctor_get(current_element, 0);
        lean_object * tail = lean_ctor_get(current_element, 1);
        uint32_t current_buffer = lean_unbox(head);
        glDeleteBuffers(1, &current_buffer);

        // advance to next element
        current_element = tail;
    }
    return lean_return_unit();
}

/**
 * inductive BufferTarget
 * | ArrayBuffer
 * | ElementBuffer
 * | TextureBuffer
 * | UniformBuffer
 */
typedef uint8_t bufferTarget_t;

//constant convertGLBufferTarget : BufferTarget → UInt32
//
uint32_t lean_convert_gl_buffer_target(bufferTarget_t leanTarget)
{
    GLenum glTarget = GL_ARRAY_BUFFER;
    switch (leanTarget)
    {
        case 0: // ArrayBuffer
            glTarget = GL_ARRAY_BUFFER;
            break;
        case 1: // ElementBuffer
            glTarget = GL_ELEMENT_ARRAY_BUFFER;
            break;
        case 2: // TextureBuffer
            glTarget = GL_TEXTURE_BUFFER;
            break;
        case 3: // UniformBuffer
            glTarget = GL_UNIFORM_BUFFER;
            break;
    }
    return glTarget;
}

//constant glBindBuffer : BufferTarget → GLBufferObject -> IO Unit
//
lean_obj_res lean_opengl_glbindbuffer(bufferTarget_t bufferTarget, uint32_t bufferName)
{
    // convert target from Lean enum to openGL C constant
    glBindBuffer(lean_convert_gl_buffer_target(bufferTarget), bufferName);
    return lean_return_unit();
}


/**
 * inductive BufferFrequency
 * | StreamBuffer   -- written once and used a few times
 * | StaticBuffer   -- written once, used many times
 * | DynamicBuffer  -- written many times, used many times
 *
 * inductive BufferAccessPattern
 * | DrawBufferAccess
 * | ReadBufferAccess
 * | CopyBufferAccess
 */
typedef uint8_t bufferFrequency_t;
typedef uint8_t bufferAccessPattern_t;

static GLenum bufferUsageTable[9] = {
    //
    // freq = StreamBuffer
    //
    GL_STREAM_DRAW,
    GL_STREAM_READ,
    GL_STREAM_COPY,
    //
    // freq = StaticBuffer
    //
    GL_STATIC_DRAW,
    GL_STATIC_READ,
    GL_STATIC_COPY,
    //
    // freq = DynamicBuffer
    //
    GL_DYNAMIC_DRAW,
    GL_DYNAMIC_READ,
    GL_DYNAMIC_COPY
};

GLenum lean_convert_gl_bufferusage(bufferFrequency_t freq, bufferAccessPattern_t access)
{
    int lookupIndex = freq * 3 + access;
    assert (lookupIndex >= 0 && lookupIndex < 9);
    return bufferUsageTable[lookupIndex];
}

// writes a lean sarray to a glBuffer. tries to convert the array size and element size to proper GL equivalents.
lean_obj_res lean_opengl_glbufferdata_sarray(
    bufferTarget_t leanTarget, 
    lean_obj_arg sArray, 
    bufferFrequency_t freq, 
    bufferAccessPattern_t access)
{
    if (!lean_is_sarray(sArray)) {
        return lean_io_result_mk_error(lean_mk_io_user_error(lean_mk_string("called glBufferData with a non-sarray object")));
    }

    uint32_t cTarget = lean_convert_gl_buffer_target(leanTarget);
    GLenum bufferUsage = lean_convert_gl_bufferusage(freq,access);

    size_t dataSize = lean_sarray_size(sArray);
    unsigned elemSize = lean_sarray_elem_size(sArray);
    uint8_t *bufferData = lean_sarray_cptr(sArray);

    glBufferData(cTarget, dataSize * elemSize, bufferData, bufferUsage);
    return lean_return_unit();
}

//constant glBufferDataBytes : BufferTarget → ByteArray → BufferFrequency → BufferAccessPattern → IO Unit
//
lean_obj_res lean_opengl_glbufferdata_bytes(
    bufferTarget_t leanTarget, 
    lean_obj_arg byteArray, 
    bufferFrequency_t freq, 
    bufferAccessPattern_t access)
{
    return lean_opengl_glbufferdata_sarray(leanTarget, byteArray, freq, access);
}

//constant glBufferDataFloats : BufferTarget → FloatArray → BufferFrequency → BufferAccessPattern → IO Unit
//
lean_obj_res lean_opengl_glbufferdata_floats(
    bufferTarget_t leanTarget, 
    lean_obj_arg floatArray, 
    bufferFrequency_t freq, 
    bufferAccessPattern_t access)
{
    return lean_opengl_glbufferdata_sarray(leanTarget, floatArray, freq, access);
}

/**
 * inductive ShaderType
 * | ComputeShader
 * | VertexShader
 * | TessellationControlShader
 * | TessellationEvaluationShader
 * | GeometryShader
 * | FragmentShader
 */
typedef uint8_t shaderType_t;


//constant glCreateShader : ShaderType → IO GLShaderObject
//
lean_obj_res lean_opengl_glcreateshader(shaderType_t shaderType)
{
    GLuint cShaderType=0;
    switch (shaderType) {
        case 0: // ComputeShader (ONLY in 4.3 or later)
          //cShaderType = GL_COMPUTE_SHADER; break;
          return lean_io_result_mk_error(lean_mk_io_user_error(lean_mk_string("Compute shader is only supported in OpenGL 4.3 or later")));
        case 1: // VertexShader
          cShaderType = GL_VERTEX_SHADER; break;
        case 2: // TessellationControlShader
          cShaderType = GL_TESS_CONTROL_SHADER; break;
        case 3: // TessellationEvaluationShader
          cShaderType = GL_TESS_EVALUATION_SHADER; break;
        case 4: // GeometryShader
          cShaderType = GL_GEOMETRY_SHADER; break;
        case 5: // FragmentShader
          cShaderType = GL_FRAGMENT_SHADER; break;
        default: // error
          return lean_io_result_mk_error(lean_mk_io_user_error(lean_mk_string("Invalid shaderType in glCreateShader")));
    }

    GLuint shaderID = glCreateShader(cShaderType);

    return lean_io_result_mk_ok(lean_box_uint64((uint64_t)shaderID));
}

//constant glShaderSource : GLShaderObject → List String → IO Unit
//
lean_obj_res lean_openl_glshadersource(uint64_t shaderID, lean_obj_arg lines)
{
    GLsizei lineCount = lean_listlength(lines);

    // alloc memory to store string pointers
    const GLchar ** lineData = calloc(lineCount, sizeof (GLchar*));

    // fill in data array
    int arrayIndex = 0;
    lean_object *current_element = lines;
    while (!lean_is_scalar(current_element)) {
        // element is a "cons head tail" object
        lean_object * head = lean_ctor_get(current_element, 0);
        lean_object * tail = lean_ctor_get(current_element, 1);

        // get string data from the head value
        if (lean_is_string(head)) {
            lineData[arrayIndex] = lean_string_cstr(head);
            // advance to next element (the tail)
            arrayIndex++;
        }
        else {
            // not a string? skip it
        }

        current_element = tail;
    }

    // at this point arrayIndex will hold the number of valid lines, ignoring invalid (non-string) elements
    if (arrayIndex < lineCount) {
        fprintf(stderr, "Invalid lines when setting shader source\n");
    }

    glShaderSource(shaderID, arrayIndex, lineData, NULL);

    free (lineData);
    
    return lean_return_unit();
}

//constant glCompileShader : GLShaderObject → IO Unit
//
lean_obj_res lean_opengl_glcompileshader(uint64_t shaderID)
{
    
    return lean_return_unit();
}

