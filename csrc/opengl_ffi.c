#include <lean/lean.h>

#include <glad/glad.h>

#include "data_marshal.h"

#include <stdio.h>
#include <stdlib.h>

void APIENTRY debugCallbackFunction(
    GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar *message,
    const void *userParam)
{
    fprintf(stderr, "OpenGL Debug Output: %s\n", message);
}

// enableGLDebugOutput : IO Unit
//
lean_obj_res lean_opengl_debugoutput()
{
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(debugCallbackFunction, NULL);
    return lean_return_unit();
}


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
    switch (leanTarget)
    {
        case 0: return GL_ARRAY_BUFFER;
        case 1: return GL_ELEMENT_ARRAY_BUFFER;
        case 2: return GL_TEXTURE_BUFFER;
        case 3: return GL_UNIFORM_BUFFER;
    }
    return GL_ARRAY_BUFFER;
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
    double *bufferData = (double *)lean_sarray_cptr(sArray);

    printf("dataSize=%d, elemSize=%d\n", dataSize,elemSize);
    for (int ix=0; ix < dataSize; ix++) {
        printf("element: %f\n", bufferData[ix]);
    }

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

//constant glBufferDataDoubles : BufferTarget → FloatArray → BufferFrequency → BufferAccessPattern → IO Unit
//
lean_obj_res lean_opengl_glbufferdata_doubles(
    bufferTarget_t leanTarget, 
    lean_obj_arg floatArray, 
    bufferFrequency_t freq, 
    bufferAccessPattern_t access)
{
    return lean_opengl_glbufferdata_sarray(leanTarget, floatArray, freq, access);
}

// glBufferDataFloats : BufferTarget → FloatArray → BufferFrequency → BufferAccessPattern → IO Unit
//
lean_obj_res lean_opengl_glbufferdata_floats(
    bufferTarget_t leanTarget, 
    lean_obj_arg floatArray, 
    bufferFrequency_t freq, 
    bufferAccessPattern_t access)
{
    // convert floatArray data from doubles to floats
    size_t arraySize = lean_sarray_size(floatArray);
    double *bufferData = (double *)lean_sarray_cptr(floatArray);
    float *truncatedElements = calloc(arraySize, sizeof(float));
    for (int ix=0; ix < arraySize; ix++) {
        truncatedElements[ix] = (float)bufferData[ix];
    }

    uint32_t cTarget = lean_convert_gl_buffer_target(leanTarget);
    GLenum bufferUsage = lean_convert_gl_bufferusage(freq,access);

    glBufferData(cTarget, arraySize * sizeof(float), truncatedElements, bufferUsage);

    free(truncatedElements);

    return lean_return_unit();
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


// glCreateShader : ShaderType → IO GLShaderObject
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


//  glDeleteShader : GLShaderObject → IO Unit
//
lean_obj_res lean_openl_gldeleteshader(uint64_t shaderID)
{
    glDeleteShader((GLuint)shaderID);
    return lean_return_unit();
}

// glShaderSource : GLShaderObject → List String → IO Unit
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

// glCompileShader : GLShaderObject → IO Unit
//
lean_obj_res lean_opengl_glcompileshader(uint64_t shaderID)
{
    glCompileShader(shaderID);

    // check for error and get the info log
    GLint compileResult;
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &compileResult);
    if (compileResult != GL_TRUE) {
        GLint logLength;
        glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &logLength);
        GLchar *logString = malloc(logLength);
        glGetShaderInfoLog(shaderID, logLength, NULL, logString);
        lean_obj_res errorLog = lean_mk_string(logString);
        free(logString);
        return lean_io_result_mk_error(lean_mk_io_user_error(errorLog));
    }

    // success
    return lean_return_unit();
}


// glCreateProgram : IO GLProgramObject
//
lean_obj_res lean_opengl_createprogram()
{
    GLuint programID = glCreateProgram();
    return lean_io_result_mk_ok(lean_box_uint64((uint64_t)programID));
}

// glDeleteProgram : GLProgramObject → IO Unit
//
lean_obj_res lean_openl_deleteprogram(uint64_t programID)
{
    glDeleteProgram((GLuint)programID);
    return lean_return_unit();
}


// glAttachShader : GLProgramObject → GLShaderObject → IO Unit
//
lean_obj_res lean_opengl_attachshader(uint64_t programID, uint64_t shaderID)
{
    glAttachShader((GLuint)programID, (GLuint)shaderID);
    return lean_return_unit();    
}

// glLinkProgram : GLProgramObject → IO Unit
//
lean_obj_res lean_opengl_linkprogram(uint64_t programID)
{
    glLinkProgram((GLuint)programID);

    // check for link error
    GLint result;
    glGetProgramiv(programID, GL_LINK_STATUS, &result);
    if (result != GL_TRUE) {
        GLint logLength;
        glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &logLength);
        GLchar *logString = malloc(logLength);
        glGetProgramInfoLog(programID, logLength, NULL, logString);
        lean_obj_res errorLog = lean_mk_string(logString);
        free(logString);
        return lean_io_result_mk_error(lean_mk_io_user_error(errorLog));
    }

    return lean_return_unit();    
}

// glUseProgram : GLProgramObject → IO Unit
//
lean_obj_res lean_opengl_useprogram(uint64_t programID)
{
    glUseProgram((GLuint)programID);
    return lean_return_unit();
}


// glGetUniformLocation : GLProgramObject → String → IO Int32
//
lean_obj_res lean_opengl_getuniformlocation(uint64_t programID, lean_obj_arg parameterName)
{
    char const *nameCStr = lean_string_cstr(parameterName);
    GLint parameterLocation = glGetUniformLocation((GLuint)programID, nameCStr);
    if (parameterLocation < 0) {
        char errorBuffer[500];
        snprintf(errorBuffer,499,"Could not find parameter name '%s' in current shader", nameCStr);
        return lean_io_result_mk_error(lean_mk_io_user_error(lean_mk_string(errorBuffer)));
    }
    return lean_io_result_mk_ok(lean_box_uint32((uint32_t)parameterLocation));
}

// glProgramUniformMatrix4fv : GLProgramObject → Uint32 -> FloatArray → IO Unit
//
lean_obj_res lean_opengl_programuniformmatrix4fv(uint64_t programID, uint32_t location, lean_obj_arg matrixData)
{
    if (lean_sarray_size(matrixData) != 16) {
        return lean_io_result_mk_error(lean_mk_io_user_error(lean_mk_string("Matrix data needs to be 16 floats in glProgramUniformMatrix4fv")));
    }

    glProgramUniformMatrix4fv(
        (GLuint)programID,
        (GLint)location,
        1,
        false,
        ((GLfloat*)lean_sarray_cptr(matrixData))
    );
    return lean_return_unit();
}





// glGenVertexArrays : UInt32 → IO (Array GLVertexArrayObject)
//
lean_obj_res lean_opengl_genvertexarrays(uint32_t count)
{
    GLuint *vaoNames = calloc(count, sizeof(GLuint));
    glGenVertexArrays(count, vaoNames);

    // copy from a C array of GLuint elements to a lean array of uint64_t elements
    lean_object *vaoArray = lean_alloc_array(count, count);
    lean_object **leanVaoNames = lean_array_cptr(vaoArray);
    for (int ix=0; ix < count; ix++) {
        leanVaoNames[ix] = lean_box_uint64((uint64_t)vaoNames[ix]);
    }

    free(vaoNames);

    return lean_io_result_mk_ok(vaoArray);
}

// glBindVertexArray : GLVertexArrayObject → IO Unit
//
lean_obj_res lean_opengl_bindvertexarray(uint64_t vaoID)
{
    glBindVertexArray((GLuint)vaoID);
    return lean_return_unit();
}

// glDeleteVertexArrays : Array GLVertexArrayObject → IO Unit
//
lean_obj_res lean_opengl_deletevertexarrays(lean_obj_arg vaoArray)
{
    lean_object **leanVaoNames = lean_array_cptr(vaoArray);
    GLsizei count = lean_array_size(vaoArray);

    // convert to a C array of GLuint values
    GLuint *vaoNames = calloc(count, sizeof(GLuint));
    for (int ix=0; ix < count; ix++) {
        vaoNames[ix] = (GLuint)lean_unbox_uint64(leanVaoNames[ix]);
    }
    glDeleteVertexArrays(count, vaoNames);
    free(vaoNames);

    return lean_return_unit();
}

// glBindVertexBuffer : (bindingIndex : UInt64) → GLBufferObject → (offset : UInt64) → (stride : UInt64) → IO Unit
//
lean_obj_res lean_opengl_glbindvertexbuffer(uint64_t bindingindex, uint64_t bufferObject, uint64_t offset, uint64_t stride)
{
    glBindVertexBuffer((GLuint)bindingindex, (GLuint)bufferObject, (GLintptr)offset, (GLsizei)stride);
    return lean_return_unit();
}

// glEnableVertexAttribArray : (attribIndex : UInt64) → IO Unit
//
lean_obj_res lean_opengl_enablevertexattribarray(uint64_t attribindex)
{
    glEnableVertexAttribArray((GLuint)attribindex);
    return lean_return_unit();
}

// glDisableVertexAttribArray : (attribIndex : UInt64) → IO Unit
//
lean_obj_res lean_opengl_disablevertexattribarray(uint64_t attribindex)
{
    glDisableVertexAttribArray((GLuint)attribindex);
    return lean_return_unit();
}

/*
inductive GLDataType where
| GLByte
| GLUnsignedByte
| GLShort
| GLUnsignedShort
| GLInt
| GLUnsignedInt
| GLFloat
| GLDouble
*/
typedef uint8_t dataType_t;

GLenum convertGLDataType(dataType_t dataType)
{
    switch (dataType)
    {
        case 0: return GL_BYTE;
        case 1: return GL_UNSIGNED_BYTE;
        case 2: return GL_SHORT;
        case 3: return GL_UNSIGNED_SHORT;
        case 4: return GL_INT;
        case 5: return GL_UNSIGNED_INT;
        case 6: return GL_FLOAT;
        case 7: return GL_DOUBLE;
    }
    // error
    return GL_BYTE;
}

// glVertexAttribFormat : (attribIndex : UInt64) → (size : UInt64) → GLDataType → (normalized : Bool) → (relativeOffset : UInt64) → IO Unit
//
lean_obj_res lean_opengl_vertexattribformat(uint64_t attribindex, uint64_t size, dataType_t dataType, uint8_t normalized, uint64_t relativeoffset)
{
    glVertexAttribFormat((GLuint)attribindex, (GLint)size, convertGLDataType(dataType), (GLboolean)normalized, (GLuint)relativeoffset);
    return lean_return_unit();
}

// glVertexAttribBinding : (attribIndex : UInt64) → (bindingIndex : UInt64) → IO Unit
//
lean_obj_res lean_opengl_vertexattribbinding(uint64_t attribindex, uint64_t bindingindex)
{
    glVertexAttribBinding((GLuint)attribindex, (GLuint)bindingindex);
    return lean_return_unit();
}


/*inductive GLDrawMode where
| GLPoints
| GLLines
| GLTriangleStrip
| GLTriangleFan
| GLTriangles
*/
typedef uint8_t glDrawMode_t;
GLenum convertGLDrawMode(glDrawMode_t mode)
{
    switch (mode)
    {
        case 0 : return GL_POINTS;
        case 1 : return GL_LINES;
        case 2 : return GL_TRIANGLE_STRIP;
        case 3 : return GL_TRIANGLE_FAN;
        case 4 : return GL_TRIANGLES;
    }

    return GL_POINTS;
}

// glDrawArrays : GLDrawMode → (first : UInt64) → (count : UInt64) → IO Unit
//
lean_obj_res lean_opengl_drawarrays(glDrawMode_t mode, uint64_t first, uint64_t count)
{
    GLenum glMode = convertGLDrawMode(mode);
    glDrawArrays(glMode, (GLint)first, (GLsizei)count);

    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        return lean_io_result_mk_error(lean_mk_io_user_error(lean_mk_string("A GL error occured in glDrawArrays")));
    }

    return lean_return_unit();
}
