Ideam
================
![Screenshot](https://raw.github.com/AmosCaster/ideam/master/data/screenshot/screenshot-0.6.1-eng.png)
    screenshot-0.6.1-eng.png

Introducing
----------------

Ideam is an IDE for [Haiku](https://www.haiku-os.org).
The editor is based on [Scintilla for Haiku](https://sourceforge.net/p/scintilla/haiku/ci/default/tree/).  
From version 0.5.5 onwards it is developed and tested on both x86 variants.


Compiling
----------------

* Install `scintilla` and `scintilla_devel` hpkgs from HaikuPorts (`scintilla_x86` versions for x86_gcc2)
* Clone `ideam` sources
* Execute `make` in Ideam's top directory  
The executable is created in `app` subdirectory.  


If you would like to try a clang++ build:
* Install `llvm_clang` hpkg from HaikuPorts
* Accord `CXX` to `clang++` in `Makefile`


License
----------------

Ideam is available under the MIT license. See [License.md](License.md).
