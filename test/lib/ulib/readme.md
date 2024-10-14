ulib
======
ulib is a small-ish general-purpose modular library written in C99. It doesn't
do much and it doesn't do it very well, but it's mine.

At present the only API documentation is found in the header files in `./include`.


Configuration
======
There's a template configuration file called `ulibconfig_template.h` in the
root directory.

By default, a configuration file called `ulibconfig.h` is searched for in the
default header search path. The macro `ULIB_CONFIG_HEADER` can be set to some
other name if desired.


Building
======
To build, run `make [debug|release]` in the root directory. This will generate
both a shared library `out/shared/libulib.so` and a static library `out/static/libulib.a`.

To build only the shared library, run `make shared-[debug|release]`.

To build only the static library, run `make static-[debug|release]`.

The environment variables `CC`, `AR`, and `SIZE` can be set to the program
to use instead of the default `cc`, `ar`, `size`.

The environment variable `CFLAGS` can be set to pass addition C compiler options.

The environment variable `LDFLAGS` can be set to pass addition linker options.


Usage
======
To use a module, enable it in the configuration file and include the header for
that module in the main code, then link against one of the libraries as usual.
