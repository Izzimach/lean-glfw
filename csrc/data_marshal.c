
#include <lean/lean.h>
#include "data_marshal.h"


//
// To use opaque pointers in FFI we need to pack them inside a lean object.
// Lean needs a class that describes how to free/clean up the pointer later.
// We handle this with a static function and data.
//

static lean_external_class *g_opaqueptr_class = NULL;

void opaque_finalize(void *p) {}
void opaque_foreach(void *mod, b_lean_obj_arg fn) {}

static lean_external_class *get_opaqueptr_class()
{
  if (g_opaqueptr_class == NULL) {
    g_opaqueptr_class = lean_register_external_class(&opaque_finalize, &opaque_foreach);
  }
  return g_opaqueptr_class;
}

extern lean_object *lean_mk_ptr(void *someptr)
{
      lean_external_class *opaqueptr_class = get_opaqueptr_class();
      lean_object *packed = lean_alloc_external(opaqueptr_class, someptr);
}

extern lean_object *lean_mk_packptr(void *someptr)
{
    if (someptr == NULL) {
      return lean_box(0);
    }
    else {
      lean_external_class *opaqueptr_class = get_opaqueptr_class();
      lean_object *packed = lean_alloc_external(opaqueptr_class, someptr);
      return lean_mk_option_some(packed);
    }
}


lean_object *lean_convert_uint32_array(unsigned int count, const uint32_t *cArray)
{
    lean_object *vaoArray = lean_alloc_array(count, count);
    lean_object **leanVaoNames = lean_array_cptr(vaoArray);
    for (int ix=0; ix < count; ix++) {
        leanVaoNames[ix] = lean_box_uint32((uint32_t)cArray[ix]);
    }
    return vaoArray;
}


