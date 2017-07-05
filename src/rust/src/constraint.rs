pub struct constraint;
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
    pub fn trigger_constraint(c: *mut constraint) -> bool;
    pub fn set_on_queue_true(c: *mut constraint);
    pub fn set_on_queue_false(c: *mut constraint);
    pub fn get_on_queue(c: *mut constraint) -> bool;
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
pub struct OverlapConstraint {
    tag: ConstraintType,
    constraint: *mut overlap_constraint
}

// uniqueness constraint
#[repr(C)]
pub struct UniquenessConstraint {
    tag: ConstraintType,
    constraint: *mut uniqueness_constraint
}

impl OverlapConstraint {
    fn new(w: *mut wordvar, l: *mut lettervar, offset: isize) -> OverlapConstraint {
        unsafe {
            let c = new_overlap_constraint(w, l, offset);
            OverlapConstraint {
                tag: ConstraintType::OVERLAP,
                constraint: c 
            }
        }
    }
}
        
impl UniquenessConstraint {
    fn new(w: *mut wordvar, other: *mut GSList) -> UniquenessConstraint {
        unsafe {
            let c = new_uniqueness_constraint(w, other);
            UniquenessConstraint {
                tag: ConstraintType::UNIQUENESS,
                constraint: c
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
            set_on_queue_true(self.constraint as *mut constraint)
        }
    }

    fn set_unqueued(&mut self) {
        unsafe {
            set_on_queue_false(self.constraint as *mut constraint)
        }
    }

    fn get_queued(&mut self) -> bool {
        unsafe {
            get_on_queue(self.constraint as *mut constraint)
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
            set_on_queue_true(self.constraint as *mut constraint)
        }
    }

    fn set_unqueued(&mut self) {
        unsafe {
            set_on_queue_false(self.constraint as *mut constraint)
        }
    }
    
    fn get_queued(&mut self) -> bool {
        unsafe {
            get_on_queue(self.constraint as *mut constraint)
        }
    }
}
       
#[no_mangle]
pub unsafe extern "C" fn make_overlap_constraint(w: *mut wordvar, l: *mut lettervar, offset: isize) -> *mut OverlapConstraint {
    Box::into_raw(Box::new(
            OverlapConstraint::new(w, l, offset)))
}

#[no_mangle]
pub unsafe extern "C" fn make_uniqueness_constraint(w: *mut wordvar, other: *mut GSList) -> *mut UniquenessConstraint {
    Box::into_raw(Box::new(
            UniquenessConstraint::new(w, other)))
}
