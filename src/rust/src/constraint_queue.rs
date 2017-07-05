use std::vec::Vec;
use std::os::raw::c_void;

pub struct constraint;

#[link(name = "arccc")]
extern {
    pub fn trigger_constraint(c: *mut constraint) -> bool;
    pub fn set_on_queue_true(c: *mut constraint);
    pub fn set_on_queue_false(c: *mut constraint);
}

// TODO: this is actually a stack; arcccc has a confusingly named 'queue' variable that is actually
// a stack, so we follow that here while we have part of the code in C and part in rust. rename
// when everything is in rust.
type Queue = Vec<*mut constraint>;

#[no_mangle]
pub extern "C" fn queue_new() -> *mut Queue {
    Box::into_raw(Box::new(Queue::new()))
}

#[no_mangle]
pub unsafe extern "C" fn run_constraint_queue(queue: *mut Queue) -> bool {
    let mut done = false;
    let mut q = Box::from_raw(queue);
    while !q.is_empty() {
        let mut c = q.pop();
        match c {
            Some(c) => {
                if !done {
                    done = !trigger_constraint(c);
                }
                set_on_queue_false(c);
            }
            None => ()
        }
    }
    Box::into_raw(q);
    !done
}

#[no_mangle]
pub unsafe extern "C" fn add_constraint_to_queue(queue: *mut Queue, c: *mut constraint) -> *mut Queue {
    let mut q = Box::from_raw(queue);
    q.push(c);
    set_on_queue_true(c);
    Box::into_raw(q)
}


        



