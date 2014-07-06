use datfile::DatFile;
use std::io::{IoResult, stdout};
use std::num::from_str_radix;

pub mod datfile;

fn ls_datfile(file: &str) -> IoResult<()> {
    let mut datfile = try!(DatFile::open(file));
    try!(datfile.iterate_files(|id, loc, size| println!("{:08x} {:08x} {}", id, loc, size)));
    Ok(())
}

fn cat_datfile(file: &str, id: u32) -> IoResult<()> {
    let mut datfile = try!(DatFile::open(file));
    let data = try!(datfile.read_file_by_id(id));
    try!(stdout().write(data.as_slice()));
    Ok(())
}

fn print_usage() {
    println!("usage: ");
    println!("  <dat file> ls");
    println!("  <dat file> cat <hex id>");
}

fn main() {
    let args = std::os::args();

    if args.len() < 3 {
        print_usage();
        return;
    }

    let datfile = args.get(1).as_slice();
    let action = args.get(2).as_slice();

    if action == "ls" {
        match ls_datfile(datfile) {
            Err(e) => fail!("ls failed: {}", e),
            _ => ()
        }
    }
    else if action == "cat" {
        if args.len() < 4 {
            print_usage();
            return;
        }
        let id = args.get(3).as_slice();
        let num_id = match from_str_radix::<u32>(id, 16) {
            Some(e) => e,
            None => { print_usage(); return }
        };
        match cat_datfile(datfile, num_id) {
            Err(e) => fail!("cat failed: {}", e),
            _ => ()
        }
    }
    else {
        print_usage();
    }
}

