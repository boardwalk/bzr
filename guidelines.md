# Code guidelines

The [Google C++ Style Guide](http://google-styleguide.googlecode.com/svn/trunk/cppguide.html) applies unless otherwise specified.

## Style

* Four space indentation.
* All blocks must have braces.
* Leave a blank line before and after blocks and between functions with bodies.
* Other blank lines are discretionary.
* The rare global variable `g_looksLikeThis`.
* Constants and enumeration values `LOOK_LIKE_THIS`.
* Template variables look like `T`, `E`
* Local variable s`lookLikeThis` (`not_like_this`).

## Accessors

* Getter methods are named like `getWidth`.
* Setter methods are named like `setHeight`.
* Methods that do any calculation should not use "get", like `calcArea`.
* Avoid getter and setter fluff where public fields suffice.

## Namespaces

* Namespaces `looklikethis`.

## Includes

* Includes are ordered by local include (e.g. foo.h for foo.cpp), project includes, library includes, then system includes.
* Prefer forward declarations over includes in header files when they are simple (e.g. `class Foo`).

## Instance construction

* Use equal sign initialization (e.g. `int bar = 3`) for fundamental types.
* Use paren init syntax (e.g. `MyObject obj(1, 2, 3)`) for class types.
* Do not use brace init syntax (e.g. `MyObject obj {1, 2, 3}`). VS2012 does not support it.

## Type inference

* Use `auto` for verbose types involving templates and unutterable types.
* Do not use `auto` otherwise!

## Miscellaneous

* Use `for(;;)` for infinite loops (VS will complain about `while(true)`)
