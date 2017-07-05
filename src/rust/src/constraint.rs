use std::os::raw::c_void;

pub struct overlap_constraint;
pub struct uniqueness_constraint;
pub struct wordvar;
pub struct lettervar;
pub struct GSList;

#[link(name = "arccc")]
extern {
    pub fn new_overlap_constraint(
        w: *mut wordvar,
        l: *mut lettervar,
        offset: isize) -> *mut overlap_constraint;
    pub fn new_uniqueness_constraint(
        w: *mut wordvar,
        other: *mut GSList) -> *mut uniqueness_constraint;
    pub fn revise_word_letter(c: *mut overlap_constraint) -> bool;
    pub fn revise_word_unique(c: *mut uniqueness_constraint) -> bool;
}

trait Constraint {
    fn run(&mut self) -> bool;
}

// word to letter constraint
#[repr(C)]
pub struct OverlapConstraint {
    constraint: *mut overlap_constraint
}

// uniqueness constraint
#[repr(C)]
pub struct UniquenessConstraint {
    constraint: *mut uniqueness_constraint
}

impl OverlapConstraint {
    fn new(w: *mut wordvar, l: *mut lettervar, offset: isize) -> OverlapConstraint {
        unsafe {
            let c = new_overlap_constraint(w, l, offset);
            OverlapConstraint { constraint: c }
        }
    }
}
        
impl UniquenessConstraint {
    fn new(w: *mut wordvar, other: *mut GSList) -> UniquenessConstraint {
        unsafe {
            let c = new_uniqueness_constraint(w, other);
            UniquenessConstraint { constraint: c }
        }
    }
}

impl Constraint for OverlapConstraint {
    fn run(&mut self) -> bool {
        unsafe {
            revise_word_letter(self.constraint)
        }
    }
}
        
impl Constraint for UniquenessConstraint {
    fn run(&mut self) -> bool {
        unsafe {
            revise_word_unique(self.constraint)
        }
    }
}
       
pub unsafe extern "C" fn make_overlap_constraint(w: *mut wordvar, l: *mut lettervar, offset: isize) -> *mut OverlapConstraint {
    Box::into_raw(Box::new(
            OverlapConstraint::new(w, l, offset)))
}

pub unsafe extern "C" fn make_uniqueness_constraint(w: *mut wordvar, other: *mut GSList) -> *mut UniquenessConstraint {
    Box::into_raw(Box::new(
            UniquenessConstraint::new(w, other)))
}
