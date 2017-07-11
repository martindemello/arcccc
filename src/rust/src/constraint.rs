pub struct constraint;

extern crate glib_sys;

use lettervar::LetterVar;
use wordvar::WordVar;
use std;

use lettervar;
use wordvar;

#[link(name = "arccc")]
extern {
    pub fn wordlist_ptr_to_index(wordlist: *mut glib_sys::GPtrArray, index: i32) -> *mut u8;
    pub fn trigger_constraint(c: *mut constraint) -> bool;
}

#[repr(u8)]
#[derive(Debug)]
pub enum ConstraintType {
    OVERLAP,
    UNIQUENESS
}

pub trait Constraint {
    fn run(&mut self) -> bool;
    fn set_queued(&mut self);
    fn set_unqueued(&mut self);
    fn get_queued(&mut self) -> bool;
}

// word to letter constraint
#[repr(C)]
pub struct overlap_constraint {
  pub on_queue: i32,
  pub w: *mut WordVar,
  pub l: *mut LetterVar,
  pub offset: i32
}

#[repr(C)]
pub struct OverlapConstraint {
    tag: ConstraintType,
    pub constraint: *mut overlap_constraint
}

// uniqueness constraint

#[repr(C)]
pub struct uniqueness_constraint {
  on_queue: i32,
  w: *mut WordVar,
  other_words: *mut glib_sys::GSList
}

#[repr(C)]
pub struct UniquenessConstraint {
    tag: ConstraintType,
    constraint: *mut uniqueness_constraint
}

impl OverlapConstraint {
    fn new(w: *mut WordVar, l: *mut LetterVar, offset: i32) -> OverlapConstraint {
        unsafe {
            OverlapConstraint {
                tag: ConstraintType::OVERLAP,
                constraint: Box::into_raw(Box::new(overlap_constraint {
                    on_queue: 0,
                    w: w,
                    l: l,
                    offset: offset
                }))
            }
        }
    }
}
        
impl UniquenessConstraint {
    fn new(w: *mut WordVar, other: *mut glib_sys::GSList) -> UniquenessConstraint {
        unsafe {
            UniquenessConstraint {
                tag: ConstraintType::UNIQUENESS,
                constraint: Box::into_raw(Box::new(uniqueness_constraint {
                    on_queue: 0,
                    w: w,
                    other_words: other
                }))
            }
        }
    }
}

impl Constraint for OverlapConstraint {
    fn run(&mut self) -> bool {
        unsafe {
            revise_word_letter(self.constraint) != 0
        }
    }

    fn set_queued(&mut self) {
        unsafe {
            (*self.constraint).on_queue = 1;
        }
    }

    fn set_unqueued(&mut self) {
        unsafe {
            (*self.constraint).on_queue = 0;
        }
    }

    fn get_queued(&mut self) -> bool {
        unsafe {
            (*self.constraint).on_queue != 0
        }
    }
}       

impl Constraint for UniquenessConstraint {
    fn run(&mut self) -> bool {
        unsafe {
            revise_word_unique(self.constraint) != 0
        }
    }
    
    fn set_queued(&mut self) {
        unsafe {
            (*self.constraint).on_queue = 1;
        }
    }

    fn set_unqueued(&mut self) {
        unsafe {
            (*self.constraint).on_queue = 0;
        }
    }

    fn get_queued(&mut self) -> bool {
        unsafe {
            (*self.constraint).on_queue != 0
        }
    }
}
       
pub unsafe fn revise_word_letter(c: *mut overlap_constraint) -> i32 {
  // This function is called only when some entry in
  // c->l->letters_allowed has changed from TRUE to FALSE.

  let w = (*c).w;
  let l = (*c).l;
  let mut possible_values = (*w).possible_values;

  // loop through the word values, removing impossible ones
  let mut i: i32 = 0;
  while i < (*possible_values).len as i32 {
      let cptr = wordlist_ptr_to_index(possible_values, i);
      let ch = *(cptr.offset((*c).offset as isize));
      if lettervar::lettervar_letter_allowed(l, ch) == 0 {
          if wordvar::wordlist_remove_index(w, i) == 0 {
              return 0;
          }
          i = i - 1;
      }
      i = i + 1;
  }

  // fail if the word list is now empty
  if (*possible_values).len > 0 {
      return 1
  } else {
      return 0
  }
}

pub unsafe fn revise_word_unique(c: *mut uniqueness_constraint) -> i32 {
    let w = (*c).w;
    let pv = (*w).possible_values;
    let unique_word = wordlist_ptr_to_index(pv, 0);

    // check that constraint should be triggered
    if (*pv).len > 1 {
        return 1;
    }

    let mut temp = (*c).other_words;
    while temp != std::ptr::null_mut() {
        let ow = (*temp).data as *mut WordVar;
        let wordlist = (*ow).possible_values;
        for i in 0 .. (*wordlist).len {
            if wordlist_ptr_to_index(wordlist, i as i32) == unique_word {
                if wordvar::wordlist_remove_index(ow, i as i32) == 0 {
                    return 0
                }
                // fail if the word list is now empty
                if (*wordlist).len == 0 {
                    return 0;
                }
                break;
            }
        }
        temp = (*temp).next;
    }

    return 1;
}

#[no_mangle]
pub unsafe extern "C" fn make_overlap_constraint(w: *mut WordVar, l: *mut LetterVar, offset: i32) -> *mut OverlapConstraint {
    Box::into_raw(Box::new(
            OverlapConstraint::new(w, l, offset)))
}

#[no_mangle]
pub unsafe extern "C" fn make_uniqueness_constraint(
    w: *mut WordVar,
    other: *mut glib_sys::GSList
    ) -> *mut UniquenessConstraint {
    Box::into_raw(Box::new(
            UniquenessConstraint::new(w, other)))
}
