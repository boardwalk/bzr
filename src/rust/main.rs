use datfile::DatFile;
use std::io::{IoResult, stdout};

pub mod datfile;

fn test_datfile() -> IoResult<()> {
    let mut datfile = try!(DatFile::open("../../data/client_portal.dat"));

    let model = try!(datfile.read_file_by_id(0x020008C8));
    try!(stdout().write(model.as_slice()));

    let mut locs = Vec::new();

    try!(datfile.iterate_files(|id, loc| {
        //println!("id={:x} loc={:x}", id, loc);
        if id >> 24 == 0xE { // UI text
            locs.push(loc);
        }
    }));

    for loc in locs.iter() {
        let data = try!(datfile.read_file_by_loc(*loc));
        try!(stdout().write(data.as_slice()));
    }

    Ok(())
}

fn main() {
    match test_datfile() {
        Err(e) => fail!("{}", e),
        _ => ()
    }
}

