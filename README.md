
[![Build Status](https://travis-ci.org/boardwalk/bzr.svg?branch=master)](https://travis-ci.org/boardwalk/bzr)

[Design document](http://ersatsz.com/2014/07/07/baelzharons-respite/)

[Technical documentation](http://ersatsz.com/2014/07/04/asherons-call/)

## System requirements

*  Linux or Mac (Windows forthcoming)
*  Drivers support OpenGL 4.1+

## Building

Build dependencies:

* A recent [GCC](https://gcc.gnu.org/) supporting C++11
* [Python](http://python.org/) 2.7 or 3.0+
* [ninja](http://martine.github.io/ninja/)

Runtime dependencies:

* [SDL](https://www.libsdl.org/) 2.0+
* Update to date [Asheron's Call data files](http://asheronscall.com/en/content/downloads)

```sh
$ ./make_ninja_build.py && ninja
```

