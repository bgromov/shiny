Shiny
=====

Project Shiny for UCS

### Build Instructions

To generate Eclipse project files, i.e. `.project` and `.cproject`:

    $ git clone https://github.com/bgromov/shiny.git -b cmake
    $ cd shiny
    $ mkdir build && cd build
    $ cmake -G"Eclipse CDT4 - Unix Makefiles" ../src

Eclipse only supports out-of-source build, thus making `build` dir out of `src` is essential.

To open the project in Eclipse:

 1. File -> Import...
 2. General / Existing Projects into Workspace
 3. Select root directory: (Browse to shiny/build)
 4. Finish
 5. Project -> Properties
 6. C/C++ General / Preprocessor Include Paths, Macros etc. / Providers
 7. Check the `CDT GCC Build Output Parser` and `CDT GCC Built-in Compiler Settings`
 8. Select `CDT GCC Built-in Compiler Settings`
 9. Uncheck `Use global provider shared between projects`
 10. Add `-std=c++11` at the end of `Command to get compiler specs`.

The steps 5 to 10 ensure the sources are indexed correctly (including C++11 symbols). It may not be the case for such a tiny project, but will help to avoid a lot of headache in future.
    
To build the project from command line simply do:

    $ cmake ../src && make

