use std::ffi::{CStr, CString};
use std::fs::File;
use std::io::BufRead;
use std::io::BufReader;
use std::os::raw::c_char;

extern crate glib;
extern crate glib_sys;
extern crate libc;

#[no_mangle]
pub fn read_words(filename: *const c_char) -> *mut glib_sys::GPtrArray {
    unsafe {
        let fname = CStr::from_ptr(filename).to_string_lossy().into_owned();
        let f = File::open(fname).unwrap();
        let file = BufReader::new(&f);
        let out = glib_sys::g_ptr_array_new();
        let chunk = glib_sys::g_string_chunk_new(1024);
        for line in file.lines() {
            let l = line.unwrap();
            let chr = l.chars().nth(0).unwrap();
            if chr == '#' {
                continue;
            }
            if l.chars().all(|c| c.is_lowercase()) { // TODO: is_ascii
                let s = CString::new(l).unwrap();
                let c = glib_sys::g_string_chunk_insert(chunk, s.as_ptr());
                glib_sys::g_ptr_array_add(out, c as *mut libc::c_void);
            }
        }
        out
    }
}
