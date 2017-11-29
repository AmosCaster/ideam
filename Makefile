################################################################################
## Ideam makefile
##

arch := $(shell getarch)

#CXX	:= clang++

NAME := Ideam

TARGET_DIR := app

TYPE := APP

APP_MIME_SIG := "application/x-vnd.Ideam-Ideam"

SRCS :=  src/IdeamApp.cpp
SRCS +=  src/IdeamNamespace.cpp
SRCS +=  src/ui/Editor.cpp
SRCS +=  src/ui/IdeamWindow.cpp
SRCS +=  src/ui/SettingsWindow.cpp
SRCS +=  src/project/NewProjectWindow.cpp
SRCS +=  src/project/Project.cpp
SRCS +=  src/helpers/TPreferences.cpp
SRCS +=  src/helpers/tabview/TabContainerView.cpp
SRCS +=  src/helpers/tabview/TabManager.cpp
SRCS +=  src/helpers/tabview/TabView.cpp



RDEFS := Ideam.rdef

LIBS = be shared translation localestub $(STDCPPLIBS)
LIBS += scintilla columnlistview tracker

# Include libraries based on arch
#ifeq ($(arch), x86_gcc2)
#LIBS +=	/boot/system/develop/lib/x86/libcolumnlistview.a
#else
#LIBS +=	/boot/system/develop/lib/libcolumnlistview.a
#endif

LIBPATHS = $(shell findpaths -a "$(arch)" B_FIND_PATH_DEVELOP_LIB_DIRECTORY)
#$(info LIBPATHS="$(LIBPATHS)")

SYSTEM_INCLUDE_PATHS = \
	$(shell findpaths -e B_FIND_PATH_HEADERS_DIRECTORY private/shared) \
	$(shell findpaths -e B_FIND_PATH_HEADERS_DIRECTORY private/interface) \
	$(shell findpaths -e B_FIND_PATH_HEADERS_DIRECTORY scintilla)


OPTIMIZE := FULL

CFLAGS := -Wall -Werror

CXXFLAGS := -std=c++11

LOCALES := en it

DEBUGGER := TRUE

## Useful for scan-build too
ifneq ($(CXX), g++)
SYSTEM_INCLUDE_PATHS += /system/develop/tools/lib/gcc/x86_64-unknown-haiku/5.4.0/include/c++/ \
	/system/develop/tools/lib/gcc/x86_64-unknown-haiku/5.4.0/include/c++/x86_64-unknown-haiku/

CFLAGS +=  -fPIC
endif

## Include the Makefile-Engine
DEVEL_DIRECTORY := \
	$(shell findpaths -r "makefile_engine" B_FIND_PATH_DEVELOP_DIRECTORY)
 include $(DEVEL_DIRECTORY)/etc/makefile-engine

$(OBJ_DIR)/%.o : %.cpp
	$(CXX) -c $< $(INCLUDES) $(CFLAGS) $(CXXFLAGS) -o "$@"
