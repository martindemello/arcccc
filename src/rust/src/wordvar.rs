use constraint::Constraint;
use constraint::OverlapConstraint;
use constraint::UniquenessConstraint;
use constraint_queue::Queue;
use lettervar::LetterVar;
use lettervar;
use std;

#[repr(C)]
pub struct wordlist;

extern crate glib_sys;

#[link(name = "arccc")]
extern {
    pub fn wordlist_ptr_to_index(w: *mut glib_sys::GPtrArray, i: i32) -> *mut u8;
    pub fn wordlist_swap_index_with_end(w: *mut glib_sys::GPtrArray, i: i32) -> *mut u8;

    pub fn put_constraint_on_queue(q: *mut Queue, c: *mut Constraint);
}

#[repr(C)]
pub struct WordVar {
    pub possible_values: *mut glib_sys::GPtrArray,
    length: i32,
    letters: *mut *mut LetterVar,
    letter_counts: *mut *mut i32,
    stack: *mut glib_sys::GPtrArray,
    orthogonal_constraints: *mut *mut OverlapConstraint,
    unique_constraint: *mut UniquenessConstraint,
    name: *mut glib_sys::GString
}

#[no_mangle]
pub unsafe extern "C" fn init_wordvars(
    glib_sys::GSList *words,
    glib_sys::GSList *letters, 
    glib_sys:GPtrArray *dictionary) {

    let mut p = words;
    while p !=  std::ptr::null_mut() {
        let w: *mut WordVar = (*p).data;
        (*w).possible_values = glib_sys::g_ptr_array_new();

        for i in 0 .. (*dictionary).len {
            let dword = wordlist_ptr_to_index(dictionary, i as i32);

      // check that the lengths match
      if (strlen(dword) != w->length) continue;

      // check that the word matches the constraints
      for (j = 0; j < w->length; j++) {
        if (w->letters[j]->letters_allowed[(guint) dword[j]] != TRUE) break;
      }
      if (j < w->length) continue;
      
      // add this word to the possible values
      g_ptr_array_add(w->possible_values, dword);
      for (j = 0; j < w->length; j++) {
        w->letter_counts[j][(guint) dword[j]]++;
      }
    }

    if (w->possible_values->len == 0) {
      printf("Die: No words for %s.\n", w->name->str);
      exit(-1);
    }
  }



}

#[no_mangle]
pub unsafe extern "C" fn wordlist_remove_index(
    queue: *mut Queue, wptr: *mut WordVar, index: i32) -> i32 {
    let mut w = Box::from_raw(wptr);
    let temp = wordlist_swap_index_with_end(w.possible_values, index);
    // loop over characters, decrementing counts in corresponding lettervar
    for i in 0 .. w.length {
        let ix = temp.offset(i as isize);
        let t = *(w.letter_counts.offset(i as isize));
        let mut lc = t.offset(*ix as isize);
        *lc = *lc - 1;
        if *lc == 0  {
            let oc = w.orthogonal_constraints.offset(i as isize);
            let c = (**oc).constraint;
            if c != std::ptr::null_mut() {
                let l = (*c).l;
                if lettervar::lettervar_letter_allowed(l, *ix) == 0 {
                    continue;
                }

                let num = lettervar::lettervar_set_letter_allowed(l, *ix, 0);
                if num == 0 {
                    Box::into_raw(w);
                    return 0
                }

                if num == 1 {
                    lettervar::set_letter(l)
                }

                put_constraint_on_queue(queue, *oc);
            }
        }
    }

    if (*w.possible_values).len == 1 {
        put_constraint_on_queue(queue, w.unique_constraint);
    }

    Box::into_raw(w);
    return 1;
}
    
