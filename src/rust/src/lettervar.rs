use constraint::OverlapConstraint;
use std::mem;
use std;
use std::ffi::CString;

extern crate glib_sys;

// constraint between a letter square and one word in that position
#[repr(C)]
pub struct LetterVar {
  letter_counts: [[i32; 256]; 2], // support for each letter from the across=0 or down=1 words
  letters_allowed: [i32; 256], // letters that can appear in this word
  num_letters_allowed: i32,
  constraints: [*mut OverlapConstraint; 2],
  stack: *mut glib_sys::GArray, // for backtracking
  name: *mut glib_sys::GString,
  pos: *mut u8
}

impl LetterVar {
  #[no_mangle]
  pub unsafe extern "C" fn new(chr: u8, p: *mut u8) -> LetterVar {
    let f = if chr == 46 { 1 } else { 0 };
    let mut out = LetterVar {
        letter_counts: [[0; 256]; 2],
        letters_allowed: [f; 256],
        num_letters_allowed: 0,
        constraints: [std::ptr::null_mut(); 2],
        stack: glib_sys::g_array_new(
            0, 0, mem::size_of::<LetterVar>() as u32),
        name: std::ptr::null_mut(),
        pos: p
    };
    out.letters_allowed[chr as usize] = 1;
    out
  }
}

#[no_mangle]
pub unsafe extern "C" fn make_lettervar(chr: u8, p: *mut u8) -> *mut LetterVar {
    let l = Box::into_raw(Box::new(LetterVar::new(chr, p)));
    l
}
    
#[no_mangle]
pub unsafe extern "C" fn lettervar_set_name(
    lptr: *mut LetterVar,
    aw: *mut glib_sys::GString, dw: *mut glib_sys::GString,
    row: i32, col: i32) {
    let mut l = Box::from_raw(lptr);
    let cstr = CString::new("").unwrap();
    let fmtstr =  CString::new("%s / %s (%d,%d)").unwrap();
    l.name = glib_sys::g_string_new(cstr.as_ptr());
    glib_sys::g_string_printf(l.name, fmtstr.as_ptr(), aw, dw, row, col);
    Box::into_raw(l);
}
    
#[no_mangle]
pub unsafe extern "C" fn lettervar_set_constraints(
    lptr: *mut LetterVar,
    oca: *mut OverlapConstraint,
    ocd: *mut OverlapConstraint
    ) {
    let mut l = Box::from_raw(lptr);
    l.constraints[0] = oca;
    l.constraints[1] = ocd;
    Box::into_raw(l);
}
    
#[no_mangle]
pub unsafe extern "C" fn set_letter(lptr: *mut LetterVar) {
    let l = Box::from_raw(lptr);
    for i in 0..256 {
        if l.letters_allowed[i] != 0 {
            *(l.pos) = i as u8;
            break
        }
    }
    Box::into_raw(l);
}

#[no_mangle]
pub unsafe extern "C" fn lettervar_letter_allowed(
    lptr: *mut LetterVar,
    i: u8) -> i32 {
    let l = Box::from_raw(lptr);
    let out = l.letters_allowed[i as usize];
    Box::into_raw(l);
    out
}

#[no_mangle]
pub unsafe extern "C" fn lettervar_set_letter_allowed(
    lptr: *mut LetterVar,
    i: u8,
    t: i32) -> i32 {
    let mut l = Box::from_raw(lptr);
    l.letters_allowed[i as usize] = t;
    if t == 0 {
        l.num_letters_allowed -= 1;
    } else {
        l.num_letters_allowed += 1;
    }
    let out = l.num_letters_allowed;
    Box::into_raw(l);
    out
}

#[no_mangle]
pub unsafe extern "C" fn lettervar_num_letters_allowed(
    lptr: *mut LetterVar) -> i32 {
    let l = Box::from_raw(lptr);
    let out = l.num_letters_allowed;
    Box::into_raw(l);
    out
}
