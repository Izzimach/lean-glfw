// some simple data marshalling between Lean and C, including:
// - pack/unpack boxed Lean values with Lean Option data types
// - pack a pointer into an option type, with nullptr mapped to Option.none
// - pack/unpack tuples

// based on code in SDL.lean by 

#include <lean/lean.h>


/**
 * Unwrap an Option of an external object as data for some
 * or NULL for none. Unsafe.
 */
static inline void *lean_option_unwrap(lean_obj_arg a) {
  if (lean_is_scalar(a)) {
    return NULL;
  } else {
    lean_object *some_val = lean_ctor_get(a, 0);
    return lean_get_external_data(some_val);      
  }
}

