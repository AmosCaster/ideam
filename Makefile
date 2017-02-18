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
SRCS +=  src/ui/IdeamWindow.cpp
SRCS +=  src/helpers/TPreferences.cpp


RDEFS := Ideam.rdef

LIBS = be shared translation localestub $(STDCPPLIBS)

# Include libraries based on arch
ifeq ($(arch), x86_gcc2)
LIBS +=	/boot/system/develop/lib/x86/libcolumnlistview.a
else
LIBS +=	/boot/system/develop/lib/libcolumnlistview.a
endif

LIBPATHS = /boot/home/config/non-packaged/develop/lib/

SYSTEM_INCLUDE_PATHS = /system/develop/headers/private/shared/ \
	/system/develop/headers/private/interface/ 

OPTIMIZE := FULL

CFLAGS := -Wall #-Werror

CXXFLAGS := -std=c++11

LOCALES := en it

DEBUGGER := TRUE


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