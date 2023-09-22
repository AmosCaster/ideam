Ideam
================
![Screenshot](https://raw.github.com/AmosCaster/ideam/master/data/screenshot/screenshot-0.7.4-eng.png)
    screenshot-0.7.4-eng.png

Introducing
----------------

Ideam is an IDE for [Haiku](https://www.haiku-os.org).
The editor is based on [Scintilla for Haiku](https://sourceforge.net/p/scintilla/haiku/ci/default/tree/).  
From version 0.5.5 onwards it is developed and tested on both x86 variants (x86_gcc2, x86_64).


Compiling
----------------

* Install `scintilla` and `scintilla_devel` hpkgs from HaikuPorts (`scintilla_x86` versions for x86_gcc2)
* Clone `ideam` sources
* Execute `make` in Ideam's top directory  
The executable is created in `app` subdirectory.  


If you would like to try a clang++ build:
* Install `llvm_clang` hpkg from HaikuPorts
* Set `BUILD_WITH_CLANG` to `1` in `Makefile`

Notice
----------------
Currently (2023) Ideam is on hold. You may have a look at [Genio] (https://github.com/Genio-The-Haiku-IDE/Genio).

License
----------------

Ideam is available under the MIT license. See [License.md](License.md).
