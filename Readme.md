Ideam
================
![Screenshot](https://raw.github.com/AmosCaster/ideam/master/data/screenshot/screenshot-0.5.2-eng.png)
    screenshot-0.5.2-eng.png

Introducing
----------------

Ideam is an IDE for [Haiku](https://www.haiku-os.org).
The editor is based on [Scintilla for Haiku](https://sourceforge.net/p/scintilla/haiku/ci/default/tree/).  
It is developed and tested on a x86_64 build.


Compiling
----------------

* Install `scintilla` and `scintilla_devel` hpkgs from HaikuPorts
* Clone `ideam` sources
* Execute `make` in Ideam's top directory

The executable is created in app subdirectory.  

If you would like to try a clang++ build:

* Install `llvm_clang` hpkg from HaikuPorts
* Uncomment `#CXX	:= clang++` from `Makefile`


License
----------------

Ideam is available under the MIT license. See [License.md](License.md).
