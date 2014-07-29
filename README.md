[Design document](http://ersatsz.com/2014/07/07/baelzharons-respite/)

[Technical documentation](http://ersatsz.com/2014/07/04/asherons-call/)

## System requirements

*  Linux, Mac or Windows
*  OpenGL 4.1+

## Building

Build dependencies:

* A recent GCC or MSVC supporting C++11
* Python 2.7 or 3.0+
* [ninja](http://martine.github.io/ninja/)

Runtime dependencies:

* SDL 2.0+
* Updated [Asheron's Call data files](http://asheronscall.com/en/content/downloads)

```sh
$ ./make_ninja_build.py && ninja
```
