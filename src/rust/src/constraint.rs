pub struct constraint;
pub struct GSList;

use lettervar::LetterVar;
use wordvar::WordVar;

#[link(name = "arccc")]
extern {
    pub fn revise_word_letter(c: *mut overlap_constraint) -> bool;
    pub fn revise_word_unique(c: *mut uniqueness_constraint) -> bool;
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
  other_words: *mut GSList
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
    fn new(w: *mut WordVar, other: *mut GSList) -> UniquenessConstraint {
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
            revise_word_letter(self.constraint)
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
            revise_word_unique(self.constraint)
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
       
#[no_mangle]
pub unsafe extern "C" fn make_overlap_constraint(w: *mut WordVar, l: *mut LetterVar, offset: i32) -> *mut OverlapConstraint {
    Box::into_raw(Box::new(
            OverlapConstraint::new(w, l, offset)))
}

#[no_mangle]
pub unsafe extern "C" fn make_uniqueness_constraint(w: *mut WordVar, other: *mut GSList) -> *mut UniquenessConstraint {
    Box::into_raw(Box::new(
            UniquenessConstraint::new(w, other)))
}
