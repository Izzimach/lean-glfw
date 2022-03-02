// some simple data marshalling between Lean and C, including:
// - pack/unpack boxed Lean values with Lean Option data types
// - pack a pointer into an option type, with nullptr mapped to Option.none
// - pack/unpack tuples

#include <lean/lean.h>

/** make a Unit return value */
static inline lean_obj_res lean_return_unit() {
  return lean_io_result_mk_ok(lean_box(0));
}

/*
// These are from SDL.lean by Anders Christiansen Sørby:
// - lean_option_unwrap (here as lean_option_unpackptr)
// - lean_mk_option_some
// - lean_mk_option_none
*/

/**
 * Build an Option.none.
 * Note that this is the same value for Unit and other constant constructors of inductives.
 */
static inline lean_object *lean_mk_option_none() {
  return lean_box(0);
}

/**
 * Build an Option.some a
 */
static inline lean_object * lean_mk_option_some(lean_object * a) {
  lean_object* tuple = lean_alloc_ctor(1, 1, 0);
  lean_ctor_set(tuple, 0, a);
  return tuple;
}

/**
 * Extract the lean object inside an Option, returning NULL if it's Option.none
 */
static inline lean_object *lean_option_unwrap(lean_obj_arg a) {
  if (lean_is_scalar(a)) {
    return NULL;
  } else {
    lean_object *some_val = lean_ctor_get(a, 0);
    return some_val;
  }
}


/**
 * Wrap a pointer in a lean Option, with NULL mapped to Option.none and
 * any other pointer mapped to an opaque Option.Some
 *
 * Not inlined since it needs to possibly initialize a static variable.
 */
lean_object *lean_mk_packptr(void *someptr);

/**
 * Unwrap an Option of an external object as data for some
 * or NULL for none. Unsafe.
 */
static inline void *lean_option_unpackptr(lean_obj_arg a) {
  if (lean_is_scalar(a)) {
    return NULL;
  } else {
    lean_object *some_val = lean_ctor_get(a, 0);
    return lean_get_external_data(some_val);      
  }
}
