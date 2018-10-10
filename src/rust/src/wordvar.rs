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
extern crate libc;

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
    words: *mut glib_sys::GSList,
    _letters: *mut glib_sys::GSList,
    dictionary: *mut glib_sys::GPtrArray) {

    let mut p = words;
    while p != std::ptr::null_mut() {
        let wptr = (*p).data as *mut WordVar; 
        let mut w = Box::from_raw(wptr);

        w.possible_values = glib_sys::g_ptr_array_new();

        for i in 0 .. (*dictionary).len {
            let dword = wordlist_ptr_to_index(dictionary, i as i32);

            // check that the lengths match
            if libc::strlen(dword as *const i8) != w.length as usize {
                continue;
            }

            // check that the word matches the constraints
            let mut flag = false;
            for j in 0 .. w.length {
                let mut letters = Box::from_raw(w.letters);
                let letter = letters.offset(j as isize);
                let index = dword.offset(j as isize) as u8;
                if lettervar::lettervar_letter_allowed(letter, index) == 0 {
                    flag = true;
                    break;
                }
            }
            if flag {
                continue;
            }

            // add this word to the possible values
            glib_sys::g_ptr_array_add(w.possible_values, dword as *mut libc::c_void);
            for j in 0 .. w.length {
                let mut lc_j_ptr = w.letter_counts.offset(j as isize);
                let dw_ptr = dword.offset(j as isize);
                let dw = *dw_ptr as isize;
                let mut lc = *lc_j_ptr;
                let mut lc_j_dw_ptr = lc.offset(dw);
                lc_j_dw_ptr.write(*lc_j_dw_ptr + 1);
            }
        }

        if (*w.possible_values).len == 0 {
            let name = std::ffi::CStr::from_ptr((*w.name).str);
            println!("Die: No words for {}", name.to_str().unwrap());
            std::process::exit(-1);
        }

        Box::into_raw(w);
        p = (*p).next;
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
    
