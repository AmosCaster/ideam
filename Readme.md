Ideam
================
![Screenshot](https://raw.github.com/AmosCaster/ideam/master/data/screenshot/screenshot-0.0.6-ita.png)
	screenshot-0.0.6-ita

Introducing
----------------

Ideam is an IDE for [Haiku](https://www.haiku-os.org).
The editor is based on [Scintilla for Haiku](https://sourceforge.net/p/scintilla/haiku/ci/default/tree/).
It is developed and tested on a x86_64 build.


Compiling
----------------

* Download [Scintilla](http://www.scintilla.org) sources
* Download [Scintilla for Haiku](https://sourceforge.net/p/scintilla/haiku/ci/default/tree/) sources
* Follow instructions there and Build it
* Place libscintilla.a in:
  `/boot/home/config/non-packaged/develop/lib`
* Place Sci_Position.h, SciLexer.h, Scintilla.h, ScintillaView.h in:
  `/boot/home/config/non-packaged/develop/headers/scintilla`
* Download Ideam sources
* Execute `make` in Ideam's top directory

The executable is created in app subdirectory.
(as of version 0.5.0 a slightly modified version of scintilla might be needed)

License
----------------

Ideam is available under the MIT license. See [License.md](License.md).
