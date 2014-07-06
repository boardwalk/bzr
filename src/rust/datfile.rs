use std::io::{File, IoResult, SeekSet, IoError, FileNotFound};
use std::vec::Vec;
use std::mem::{size_of, transmute};

static SECTOR_SIZE_LOC: i64 = 0x144;
static ROOT_DIR_LOC_LOC: i64 = 0x160;
static NUM_FILES_OFF: uint = 0x3E;

pub struct DatFile {
    file: Box<File>,
    sector_size: uint,
    root_dir_loc: i64
}

impl DatFile {
    pub fn open(path: &str) -> IoResult<DatFile> {
        let file = try!(File::open(&Path::new(path)));
        DatFile::from_file(box file)
    }

    pub fn from_file(mut file: Box<File>) -> IoResult<DatFile> {
        try!(file.seek(SECTOR_SIZE_LOC, SeekSet));
        let sector_size = try!(file.read_le_u32()) as uint - size_of::<u32>(); // exclude next sector loc

        try!(file.seek(ROOT_DIR_LOC_LOC, SeekSet));
        let root_dir_loc = try!(file.read_le_u32()) as i64;

        Ok(DatFile {
            file: file,
            sector_size: sector_size,
            root_dir_loc: root_dir_loc
        })
    }

    /// Read a file by its identifier.
    pub fn read_file_by_id(&mut self, search_id: u32) -> IoResult<Vec<u8>> {
        let (loc, size) = try!(self.find_file(search_id));
        let mut result = try!(self.read_file_by_loc(loc));
        while result.len() > size { result.pop(); };
        Ok(result)
    }

    /// Read a file by its location.
    ///
    /// The size of the result is rounded up to the nearest sector size,
    /// since the actual length is in the directory.
    pub fn read_file_by_loc(&mut self, loc: i64) -> IoResult<Vec<u8>> {
        let mut result = Vec::new();
        let mut next_loc = loc;

        while next_loc != 0 {
            try!(self.file.seek(next_loc, SeekSet));
            next_loc = try!(self.file.read_le_u32()) as i64;

            let sector = try!(self.file.read_exact(self.sector_size));
            result.push_all(sector.as_slice());
        }

        Ok(result)
    }

    pub fn find_file(&mut self, search_id: u32) -> IoResult<(i64, uint)> {
        let mut dir_loc = self.root_dir_loc;
        loop {
            let dir = try!(self.read_file_by_loc(dir_loc));
            let words : &[u32] = unsafe { transmute(dir.as_slice()) };
            let num_files = words[NUM_FILES_OFF] as uint;

            for i in range(0, num_files) {
                let offset = NUM_FILES_OFF + 1 + i * 6;
                let id = words[offset + 1];
                let loc = words[offset + 2] as i64;
                let size = words[offset + 3] as uint;

                if search_id > id { continue; }
                if search_id == id { return Ok((loc, size)); }

                if words[0] == 0 {
                    return Err(IoError {
                        kind: FileNotFound,
                        desc: "Name specified not found in dat file",
                        detail: None
                    });
                }

                dir_loc = words[i] as i64;
                break;
            }
        }
    }

    /// Calls `f` with the id, location, and size of each file. 
    pub fn iterate_files(&mut self, f: |u32, i64, uint|) -> IoResult<()> {
        let dir_loc = self.root_dir_loc;
        self.inner_iterate_files(dir_loc, f)
    }

    fn inner_iterate_files(&mut self, dir_loc: i64, f: |u32, i64, uint|) -> IoResult<()> {
        let dir = try!(self.read_file_by_loc(dir_loc));
        let words : &[u32] = unsafe { transmute(dir.as_slice()) };
        let num_files = words[NUM_FILES_OFF] as uint;

        for i in range(0, num_files) {
            let offset = NUM_FILES_OFF + 1 + i * 6;
            let id = words[offset + 1];
            let loc = words[offset + 2] as i64;
            let size = words[offset + 3] as uint;

            if words[0] != 0 {
                try!(self.inner_iterate_files(words[i] as i64, |id, loc, size| f(id, loc, size)));
            }

            f(id, loc, size);
        }

        Ok(())
    }
}

