use std::ffi::CStr;
use std::fs::File;
use std::io::BufRead;
use std::io::BufReader;
use std::os::raw::c_char;
use std::string::String;
use std::vec::Vec;

extern crate glib;
extern crate glib_sys;
extern crate libc;

pub type Dictionary = Vec<*const String>;    
        

pub fn read_from_file(fname: &str) -> Dictionary {
    let f = File::open(fname).unwrap();
    let file = BufReader::new(&f);
    let mut out = Dictionary::new();
    for line in file.lines() {
        let l = line.unwrap();
        let chr = l.chars().nth(0).unwrap();
        if chr == '#' {
            continue;
        }
        if l.chars().all(|c| c.is_lowercase()) { // TODO: is_ascii
            out.push(Box::into_raw(Box::new(l)));
        }
    }
    out
}

#[no_mangle]
pub unsafe extern "C" fn read_words(filename: *const c_char) -> *mut Dictionary {
    let fname = CStr::from_ptr(filename).to_string_lossy().into_owned();
    let dict = read_from_file(&fname);
    return Box::into_raw(Box::new(dict))
}
