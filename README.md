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

To build the project from command line simply do:

    $ make

