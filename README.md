[![Build Status](https://travis-ci.org/boardwalk/bzr.svg?branch=master)](https://travis-ci.org/boardwalk/bzr)

[Design document](http://ersatsz.com/2014/07/07/baelzharons-respite/)

[Technical documentation](http://ersatsz.com/2014/07/04/asherons-call/)

## System requirements

[![Join the chat at https://gitter.im/boardwalk/bzr](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/boardwalk/bzr?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

*  Linux, Mac or Windows
*  OpenGL 4.1+

## Building

### Build dependencies

* A recent GCC or VC2013 or newer
* Python 2.7 or 3.0+
* [Ninja](http://martine.github.io/ninja/)
* [Jansson](http://www.digip.org/jansson/), developed with v2.6
* [glm](http://glm.g-truc.net/), developed with v0.9.5.4

#### Compiling Jansson on Windows

1. Extract Jansson to your Documents folder.
2. Install CMake.
3. Start a Visual Studio x64 command line.
4. Create a VisualStudioSolution folder in your Jansson folder.
5. Run "cmake .." in this folder.
6. Open jansson.sln that gets generated.
7. Open Build / Build Configuration... and create a x64 solution platform.
8. Make Release x64 your active configuration and build the jansson project.

### Runtime dependencies

* [SDL2](http://libsdl.org/), developed with v2.0.3
* [GLEW](http://glew.sourceforge.net/) on Windows, developed with v1.10.0
* Updated [Asheron's Call data files](http://asheronscall.com/en/content/downloads)

```sh
$ ./make_ninja_build.py && ninja
```
