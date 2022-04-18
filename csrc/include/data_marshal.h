// some simple data marshalling between Lean and C, including:
// - pack/unpack boxed Lean values with Lean Option data types
// - pack a pointer into an option type, with nullptr mapped to Option.none
// - pack/unpack tuples

#include <lean/lean.h>

/** make a Unit return valueas an IO result.
 *  For the many functions that return 'IO Unit' */
static inline lean_obj_res lean_return_unit() {
  return lean_io_result_mk_ok(lean_box(0));
}

/*
// These are Copyright 2021 by Anders Christiansen Sørby, from SDL.lean
// under the MIT license:
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
 * Returns an opaque pointer to lean. Unlike lean_mk_packptr, Lean can't tell if it's null or not.
 * Extract this with lean_get_external_data
 */
lean_object *lean_mk_ptr(void *someptr);

/**
 * Unwrap an Option of an external object as data for some
 * or NULL for none.
 */
static inline void *lean_option_unpackptr(lean_obj_arg a) {
  if (lean_is_scalar(a)) {
    return NULL;
  } else {
    lean_object *some_val = lean_ctor_get(a, 0);
    return lean_get_external_data(some_val);      
  }
}

static inline uint32_t lean_listlength(lean_obj_arg list) {
    uint32_t elementCount = 0;  
    
    lean_object *current_element = list;
    while (!lean_is_scalar(current_element)) {
        // element is a "cons head tail" object, we'll ignore the head
        //lean_object * head = lean_ctor_get(current_element, 0);
        lean_object * tail = lean_ctor_get(current_element, 1);

        // advance to next element
        elementCount++;
        current_element = tail;
    }

    return elementCount;
}

// convert a C array of uint32_t elements to a (newly-created) lean array
lean_object *lean_convert_uint32_array(unsigned int count, const uint32_t *cArray);
