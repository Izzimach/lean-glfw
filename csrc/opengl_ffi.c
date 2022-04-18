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

//def GLBufferObject := UInt32

typedef uint32_t bufferObject_t;

// glCreateBuffers : (count : UInt32) → IO (List GLBufferObject)
//
lean_obj_res lean_opengl_glcreatebuffers(uint32_t bufferCount)
{
    GLuint *bufferData = calloc(bufferCount, sizeof(GLuint));
    glCreateBuffers(bufferCount, bufferData);

    // build a Lean list from the results - note we put the elements into list backwards, but this
    // shouldn't be a problem.
    lean_object *resultList = lean_box(0); // List.nil
    for (int bix=0; bix < bufferCount; bix++)
    {
      // build a cons node and prepend to the current resultList
      lean_object *node = lean_alloc_ctor(1,2,0); // List.cons
      lean_ctor_set(node, 0, lean_box((bufferObject_t)(bufferData[bix]))); // head
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
        bufferObject_t current_buffer = lean_unbox(head);
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
lean_obj_res lean_opengl_glbindbuffer(bufferTarget_t bufferTarget, bufferObject_t bufferName)
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

/*
inductive BufferStorageFlags
| GLDynamicStorage
| GLMapRead
| GLMapWrite
| GLMapPersistent
| GLMapCoherent
| GLClientStorage
*/
typedef uint8_t storageFlags_t;

GLbitfield processStorageFlags(lean_obj_arg flagList)
{
    GLbitfield flagBits = 0;
    // walk the list
    lean_object *current_element = flagList;
    while (!lean_is_scalar(current_element)) {
        // element is a "cons head tail" object
        lean_object * head = lean_ctor_get(current_element, 0);
        lean_object * tail = lean_ctor_get(current_element, 1);
        uint8_t current_flag = (uint8_t)lean_unbox(head);
        switch (current_flag)
        {
            case 0: flagBits |= GL_DYNAMIC_STORAGE_BIT; break;
            case 1: flagBits |= GL_MAP_READ_BIT; break;
            case 2: flagBits |= GL_MAP_WRITE_BIT; break;
            case 3: flagBits |= GL_MAP_PERSISTENT_BIT; break;
            case 4: flagBits |= GL_MAP_COHERENT_BIT; break;
            case 5: flagBits |= GL_CLIENT_STORAGE_BIT; break;
        }

        // advance to next element
        current_element = tail;
    }
    
    return flagBits;
}

// glNamedBufferStorage_Floats : GLBufferObject → FloatArray → List BufferStorageFlags → IO Unit
//
lean_obj_res lean_opengl_glnamedbufferstorage_floats(bufferObject_t bufferObject, lean_obj_arg floatArray, lean_obj_arg storageFlags)
{
    GLbitfield flags = processStorageFlags(storageFlags);

    // convert floatArray data from doubles to floats
    size_t arraySize = lean_sarray_size(floatArray);
    double *bufferData = (double *)lean_sarray_cptr(floatArray);
    float *truncatedElements = calloc(arraySize, sizeof(float));
    for (int ix=0; ix < arraySize; ix++) {
        truncatedElements[ix] = (float)bufferData[ix];
    }

    glNamedBufferStorage(bufferObject, arraySize * sizeof(float), truncatedElements, flags);

    free(truncatedElements);

    return lean_return_unit();
}

// glNamedBufferStorage_Doubles : GLBufferObject → FloatArray → List BufferStorageFlags → IO Unit
//
lean_obj_res lean_opengl_glnamedbufferstorage_doubles(bufferObject_t bufferObject, lean_obj_arg doubleArray, lean_obj_arg storageFlags)
{
    GLbitfield flags = processStorageFlags(storageFlags);

    double *bufferData = (double *)lean_sarray_cptr(doubleArray);
    size_t arraySize = lean_sarray_size(doubleArray);

    glNamedBufferStorage(bufferObject, arraySize * sizeof(double), bufferData, flags);

    return lean_return_unit();
}

// glNamedBufferStorage_UInt32 : GLBufferObject → Array UInt32 → List BufferStorageFlags → IO Unit
//
lean_obj_res lean_opengl_glNamedBufferStorage_uint32(bufferObject_t bufferObject, lean_obj_arg uint32Array, lean_obj_arg storageFlags)
{
    GLbitfield flags = processStorageFlags(storageFlags);

    // unpack boxed integer values
    size_t arraySize = lean_array_size(uint32Array);
    lean_object **arrayElements = lean_array_cptr(uint32Array);
    uint32_t *unboxedElements = calloc(arraySize, sizeof(uint32_t));
    for (int ix=0; ix < arraySize; ix++) {
        unboxedElements[ix] = (uint32_t)lean_unbox_uint32(arrayElements[ix]);
    }

    glNamedBufferStorage(bufferObject, arraySize * sizeof(uint32_t), unboxedElements, flags);

    free(unboxedElements);

    return lean_return_unit();
}

// glNamedBufferStorage_Bytes : GLBufferObject → ByteArray → List BufferStorageFlags → IO Unit
//
lean_obj_res lean_opengl_glnamedbufferstorage_bytes(bufferObject_t bufferObject, lean_obj_arg byteArray, lean_obj_arg storageFlags)
{
    GLbitfield flags = processStorageFlags(storageFlags);

    uint8_t *bufferData = (uint8_t *)lean_sarray_cptr(byteArray);
    size_t arraySize = lean_sarray_size(byteArray);

    glNamedBufferStorage(bufferObject, arraySize, bufferData, flags);

    return lean_return_unit();
}




//def GLShaderObject := UInt64
typedef uint32_t glShaderObject_t;


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

    return lean_io_result_mk_ok(lean_box_uint32((glShaderObject_t)shaderID));
}


//  glDeleteShader : GLShaderObject → IO Unit
//
lean_obj_res lean_openl_gldeleteshader(glShaderObject_t shaderID)
{
    glDeleteShader((GLuint)shaderID);
    return lean_return_unit();
}

// glShaderSource : GLShaderObject → List String → IO Unit
//
lean_obj_res lean_openl_glshadersource(glShaderObject_t shaderID, lean_obj_arg lines)
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
lean_obj_res lean_opengl_glcompileshader(glShaderObject_t shaderID)
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


// def GLProgramObject := UInt32
typedef uint32_t glProgramObject_t;

// glCreateProgram : IO GLProgramObject
//
lean_obj_res lean_opengl_createprogram()
{
    GLuint programID = glCreateProgram();
    return lean_io_result_mk_ok(lean_box_uint32((glProgramObject_t)programID));
}

// glDeleteProgram : GLProgramObject → IO Unit
//
lean_obj_res lean_openl_deleteprogram(glProgramObject_t programID)
{
    glDeleteProgram((GLuint)programID);
    return lean_return_unit();
}


// glAttachShader : GLProgramObject → GLShaderObject → IO Unit
//
lean_obj_res lean_opengl_attachshader(glProgramObject_t programID, glShaderObject_t shaderID)
{
    glAttachShader((GLuint)programID, (GLuint)shaderID);
    return lean_return_unit();    
}

// glLinkProgram : GLProgramObject → IO Unit
//
lean_obj_res lean_opengl_linkprogram(glProgramObject_t programID)
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
lean_obj_res lean_opengl_useprogram(glProgramObject_t programID)
{
    glUseProgram((GLuint)programID);
    return lean_return_unit();
}


// glGetUniformLocation : GLProgramObject → String → IO Int32
//
lean_obj_res lean_opengl_getuniformlocation(glProgramObject_t programID, lean_obj_arg parameterName)
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
lean_obj_res lean_opengl_programuniformmatrix4fv(glProgramObject_t programID, uint32_t location, lean_obj_arg matrixData)
{
    if (lean_sarray_size(matrixData) != 16) {
        return lean_io_result_mk_error(lean_mk_io_user_error(lean_mk_string("Matrix data needs to be 16 floats in glProgramUniformMatrix4fv")));
    }

    // lean FloatArray is actually doubles, so we need to convert to floats
    GLfloat transferBuffer[16];
    double *sourceData = (double *)lean_sarray_cptr(matrixData);
    for (int ix=0; ix < 16; ix++) {
        transferBuffer[ix] = sourceData[ix];
    }
    
    glProgramUniformMatrix4fv(
        (GLuint)programID,
        (GLint)location,
        1,
        false,
        transferBuffer
    );
    return lean_return_unit();
}



// def GLVertexArrayObject := UInt32
typedef uint32_t vertexArrayObject_t;

// glGenVertexArrays : UInt32 → IO (Array GLVertexArrayObject)
//
lean_obj_res lean_opengl_genvertexarrays(uint32_t count)
{
    GLuint *vaoNames = calloc(count, sizeof(GLuint));
    glGenVertexArrays(count, vaoNames);

    // copy from a C array of GLuint elements to a lean array of uint32_t elements
    lean_object *vaoArray = lean_alloc_array(count, count);
    lean_object **leanVaoNames = lean_array_cptr(vaoArray);
    for (int ix=0; ix < count; ix++) {
        leanVaoNames[ix] = lean_box_uint32((vertexArrayObject_t)vaoNames[ix]);
    }

    free(vaoNames);

    return lean_io_result_mk_ok(vaoArray);
}

// glCreateVertexArrays : UInt32 → IO (Array GLVertexArrayObject)
//
lean_obj_res lean_opengl_createvertexarrays(uint32_t count)
{
    GLuint *vaoNames = calloc(count, sizeof(GLuint));
    glCreateVertexArrays(count, vaoNames);

    // copy from a C array of GLuint elements to a lean array of uint32_t elements
    lean_object *vaoArray = lean_alloc_array(count, count);
    lean_object **leanVaoNames = lean_array_cptr(vaoArray);
    for (int ix=0; ix < count; ix++) {
        leanVaoNames[ix] = lean_box_uint32((vertexArrayObject_t)vaoNames[ix]);
    }

    free(vaoNames);

    return lean_io_result_mk_ok(vaoArray);
}

// glBindVertexArray : GLVertexArrayObject → IO Unit
//
lean_obj_res lean_opengl_bindvertexarray(vertexArrayObject_t vaoID)
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
        vaoNames[ix] = (GLuint)lean_unbox_uint32(leanVaoNames[ix]);
    }
    glDeleteVertexArrays(count, vaoNames);
    free(vaoNames);

    return lean_return_unit();
}

// glBindVertexBuffer : (bindingIndex : UInt32) → GLBufferObject → (offset : UInt64) → (stride : UInt64) → IO Unit
//
lean_obj_res lean_opengl_glbindvertexbuffer(uint32_t bindingindex, uint64_t bufferObject, uint64_t offset, uint64_t stride)
{
    glBindVertexBuffer((GLuint)bindingindex, (GLuint)bufferObject, (GLintptr)offset, (GLsizei)stride);
    return lean_return_unit();
}

// glVertexArrayVertexBuffer : GLVertexArrayObject → (bindingIndex : UInt32) → GLBufferObject → (offset : UInt64) → (stride : UInt64) → IO Unit
//
lean_obj_res lean_opengl_glvertexarrayvertexbuffer(vertexArrayObject_t vao, uint32_t bindingindex, uint64_t bufferObject, uint64_t offset, uint64_t stride)
{
    glVertexArrayVertexBuffer((GLuint)vao, (GLuint)bindingindex, (GLuint)bufferObject, (GLintptr)offset, (GLsizei)stride);
    return lean_return_unit();
}


// glEnableVertexAttribArray : (attribIndex : UInt32) → IO Unit
//
lean_obj_res lean_opengl_enablevertexattribarray(uint32_t attribindex)
{
    glEnableVertexAttribArray((GLuint)attribindex);
    return lean_return_unit();
}

// glDisableVertexAttribArray : (attribIndex : UInt32) → IO Unit
//
lean_obj_res lean_opengl_disablevertexattribarray(uint32_t attribindex)
{
    glDisableVertexAttribArray((GLuint)attribindex);
    return lean_return_unit();
}

// glEnableVertexArrayAttrib : GLVertexArrayObject → (attribIndex : UInt32) → IO Unit
//
lean_obj_res lean_opengl_enablevertexarrayattrib(vertexArrayObject_t vao, uint32_t attribindex)
{
    glEnableVertexArrayAttrib((GLuint)vao, (GLuint)attribindex);
    return lean_return_unit();
}

// glDisableVertexArrayAttrib : GLVertexArrayObject → (attribIndex : UInt32) → IO Unit
//
lean_obj_res lean_opengl_disablevertexarrayattrib(vertexArrayObject_t vao, uint32_t attribindex)
{
    glDisableVertexArrayAttrib((GLuint)vao, (GLuint)attribindex);
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

// glVertexAttribFormat : (attribIndex : UInt32) → (size : UInt64) → GLDataType → (normalized : Bool) → (relativeOffset : UInt64) → IO Unit
//
lean_obj_res lean_opengl_vertexattribformat(uint32_t attribindex, uint64_t size, dataType_t dataType, uint8_t normalized, uint64_t relativeoffset)
{
    glVertexAttribFormat((GLuint)attribindex, (GLint)size, convertGLDataType(dataType), (GLboolean)normalized, (GLuint)relativeoffset);
    return lean_return_unit();
}


// glVertexArrayAttribFormat : GLVertexArrayObject → (attribIndex : UInt32) → (size : UInt64) → GLDataType → (normalized : Bool) → (relativeOffset : UInt64) → IO Unit
//
lean_obj_res lean_opengl_vertexarrayattribformat(vertexArrayObject_t vao, uint32_t attribindex, uint64_t size, dataType_t dataType, uint8_t normalized, uint64_t relativeoffset)
{
    glVertexArrayAttribFormat((GLuint)vao, (GLuint)attribindex, (GLint)size, convertGLDataType(dataType), (GLboolean)normalized, (GLuint)relativeoffset);
    return lean_return_unit();
}

// glVertexAttribBinding : (attribIndex : UInt32) → (bindingIndex : UInt32) → IO Unit
//
lean_obj_res lean_opengl_vertexattribbinding(uint32_t attribindex, uint32_t bindingindex)
{
    glVertexAttribBinding((GLuint)attribindex, (GLuint)bindingindex);
    return lean_return_unit();
}

// glVertexArrayAttribBinding : GLVertexArrayObject → (attribIndex : UInt32) → (bindingIndex : UInt32) → IO Unit
//
lean_obj_res lean_opengl_vertexarrayattribbinding(vertexArrayObject_t vao, uint32_t attribindex, uint32_t bindingindex)
{
    glVertexArrayAttribBinding((GLuint)vao, (GLuint)attribindex, (GLuint)bindingindex);
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


//def GLTextureObject := UInt32
typedef uint32_t glTextureObject_t;

/*inductive GLTextureTarget
| GLTexture1D
| GLTexture2D
| GLTexture3D
*/
typedef uint8_t glTextureTarget_t;


GLenum convertGLTextureTarget(glTextureTarget_t tt)
{
    switch (tt)
    {
        case 0: return GL_TEXTURE_1D;
        case 1: return GL_TEXTURE_2D;
        case 2: return GL_TEXTURE_3D;
    }
    return GL_INVALID_VALUE;
}

// glCreateTextures : GLTextureTarget → (count : UInt32) → IO (Array GLTextureObject)
//
lean_obj_res lean_opengl_createtextures(glTextureTarget_t leanTarget, uint32_t count)
{
    GLenum cTarget = convertGLTextureTarget(leanTarget);
    GLuint *textures = calloc(count, sizeof(GLuint));
    glCreateTextures(cTarget, count, textures);

    lean_object *leanTextures = lean_convert_uint32_array(count, textures);

    free(textures);

    return lean_io_result_mk_ok(leanTextures);
}

// glDeleteTextures : Array GLTextureObject → IO Unit
//
lean_obj_res lean_opengl_deletetextures(lean_obj_arg textureArray)
{
    GLsizei textureCount = lean_array_size(textureArray);
    GLuint *textures = calloc(textureCount, sizeof(GLuint));

    for (int ix=0; ix < textureCount; ix++) {
        textures[ix] = (uint32_t)lean_unbox_uint32(lean_array_get_core(textureArray, ix));
    }

    glDeleteTextures(textureCount, textures);

    free(textures);

    return lean_return_unit();
}

/*inductive GLSizedTextureFormat
  | R8
  | RG8
  | RGB8
  | RGBA8
  | R16F
  | R32F
*/
typedef uint8_t glSizedTextureFormat_t;

GLenum convertSizedTextureFormat(glSizedTextureFormat_t fmt)
{
    switch (fmt)
    {
        case 0: return GL_R8;
        case 1: return GL_RG8;
        case 2: return GL_RGB8;
        case 3: return GL_RGBA8;
        case 4: return GL_R16F;
        case 5: return GL_R32F;
    }
    return GL_INVALID_VALUE;
}


// glTextureStorage2D : GLTextureObject → (levels : UInt32) → GLSizedTextureFormat → (width : UInt32) → (height : UInt32) → IO Unit
//
lean_obj_res lean_opengl_texturestorage2d(
    glTextureObject_t textureObject, uint32_t levels, glSizedTextureFormat_t fmt,
    uint32_t width, uint32_t height)
{
    glTextureStorage2D((GLuint)textureObject, (GLsizei)levels, convertSizedTextureFormat(fmt), (GLsizei)width, (GLsizei)height);

    return lean_return_unit();
}

/*inductive GLPixelFormat
| Red
| RG
| RGB
| BGR
| RGBA
| BGRA
| Depth
| Stencil
*/
typedef uint8_t glPixelFormat_t;

GLenum convertPixelFormat(glPixelFormat_t fmt)
{
    switch (fmt)
    {
        case 0: return GL_RED;
        case 1: return GL_RG;
        case 2: return GL_RGB;
        case 3: return GL_BGR;
        case 4: return GL_RGBA;
        case 5: return GL_BGRA;
        case 6: return GL_DEPTH_COMPONENT;
        case 7: return GL_STENCIL_INDEX;
    }
    return GL_INVALID_VALUE;
}


/*inductive GLPixelType
| UByte
| Byte
| UShort
| Short
| UInt
| Int
| Float
*/
typedef uint8_t glPixelType_t;

GLenum convertPixelType(glPixelType_t pType)
{
    switch (pType)
    {
        case 0: return GL_UNSIGNED_BYTE;
        case 1: return GL_BYTE;
        case 2: return GL_UNSIGNED_SHORT;
        case 3: return GL_SHORT;
        case 4: return GL_UNSIGNED_INT;
        case 5: return GL_INT;
        case 6: return GL_FLOAT;
    }
    return GL_INVALID_VALUE;
}


// glTextureSubImage2D : GLTextureObject → (level : Int32) → (xoffset : Int32) → (yoffset : Int32) → (width : Int32) → (height :  Int32) → GLPixelFormat → GLPixelType → ByteArray → IO Unit
//
lean_obj_res lean_opengl_texturesubimage2d(
    // xoffset and yoffset are int, not uint, and so need to be unboxed
    glTextureObject_t textureObject, uint32_t level, lean_obj_arg xoffsetL, lean_obj_arg yoffsetL,
    uint32_t width, uint32_t height, glPixelFormat_t pixelFormat, glPixelType_t pixelType,
    lean_obj_arg pixelData)
{
    int xoffset = lean_unbox(xoffsetL);
    int yoffset = lean_unbox(yoffsetL);

    fprintf(stderr, "textureObject=%d\n", textureObject);
    fprintf(stderr, "level=%d\n", level);
    fprintf(stderr, "xoffset=%d\n", xoffset);
    fprintf(stderr, "width=%d\n", width);


    glTextureSubImage2D(
        (GLuint)textureObject,
        (GLint)level,
        (GLint)xoffset,
        (GLint)yoffset,
        (GLsizei)width,
        (GLsizei)height,
        convertPixelFormat(pixelFormat),
        convertPixelType(pixelType),
        lean_sarray_cptr(pixelData)
    );

    return lean_return_unit();
}

// glBindTextureUnit : (unit : UInt32) → GLTextureObject → IO Unit
//
lean_obj_res lean_opengl_bindtextureunit(uint32_t unit, glTextureObject_t textureObject)
{
    glBindTextureUnit((GLuint)unit, (GLuint)textureObject);
    return lean_return_unit();
}
