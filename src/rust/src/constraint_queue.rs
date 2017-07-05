use std::vec::Vec;
use constraint::{Constraint, ConstraintType, OverlapConstraint, UniquenessConstraint};

#[link(name = "arccc")]
extern {
    pub fn get_tag(c: *mut Constraint) -> ConstraintType;
}

// TODO: this is actually a stack; arcccc has a confusingly named 'queue' variable that is actually
// a stack, so we follow that here while we have part of the code in C and part in rust. rename
// when everything is in rust.
type Queue = Vec<*mut Constraint>;

#[no_mangle]
pub extern "C" fn queue_new() -> *mut Queue {
    Box::into_raw(Box::new(Queue::new()))
}

#[no_mangle]
pub unsafe extern "C" fn run_constraint_queue(queue: *mut Queue) -> bool {
    let mut done = false;
    let mut q = Box::from_raw(queue);
    while !q.is_empty() {
        let cptr = q.pop().unwrap();
        let mut c: Box<Constraint> = match get_tag(cptr) {
            ConstraintType::OVERLAP => {
                Box::from_raw(cptr as *mut OverlapConstraint)
            }
            ConstraintType::UNIQUENESS => {
                Box::from_raw(cptr as *mut UniquenessConstraint)
            }
        };
        if !done {
            done = !c.run();
        };
        c.set_unqueued();
        Box::into_raw(c);
    }
    Box::into_raw(q);
    !done
}

#[no_mangle]
pub unsafe extern "C" fn add_constraint_to_queue(queue: *mut Queue, cptr: *mut Constraint) -> *mut Queue {
    let mut q = Box::from_raw(queue);
    let mut c: Box<Constraint> = match get_tag(cptr) {
        ConstraintType::OVERLAP => {
            Box::from_raw(cptr as *mut OverlapConstraint)
        }
        ConstraintType::UNIQUENESS => {
            Box::from_raw(cptr as *mut UniquenessConstraint)
        }
    };
    if !c.get_queued() {
        q.push(cptr);
        c.set_queued();
    }
    Box::into_raw(c);
    Box::into_raw(q)
}
