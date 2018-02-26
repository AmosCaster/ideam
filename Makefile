################################################################################
## Ideam makefile
##

arch := $(shell getarch)
platform := $(shell uname -p)

ifeq ($(arch), x86_gcc2)
CC   := gcc-x86
CXX  := g++-x86
#CXX  := clang++
else
#CXX  := clang++
endif

NAME := Ideam

TARGET_DIR := app

TYPE := APP

APP_MIME_SIG := "application/x-vnd.Ideam-Ideam"

SRCS :=  src/IdeamApp.cpp
SRCS +=  src/IdeamNamespace.cpp
SRCS +=  src/ui/Editor.cpp
SRCS +=  src/ui/IdeamWindow.cpp
SRCS +=  src/ui/SettingsWindow.cpp
SRCS +=  src/project/AddToProjectWindow.cpp
SRCS +=  src/project/NewProjectWindow.cpp
SRCS +=  src/project/Project.cpp
SRCS +=  src/project/ProjectParser.cpp
SRCS +=  src/project/ProjectSettingsWindow.cpp
SRCS +=  src/helpers/IdeamCommon.cpp
SRCS +=  src/helpers/TPreferences.cpp
SRCS +=  src/helpers/console_io/ConsoleIOView.cpp
SRCS +=  src/helpers/console_io/ConsoleIOThread.cpp
SRCS +=  src/helpers/console_io/GenericThread.cpp
SRCS +=  src/helpers/tabview/TabContainerView.cpp
SRCS +=  src/helpers/tabview/TabManager.cpp
SRCS +=  src/helpers/tabview/TabView.cpp


RDEFS := Ideam.rdef

LIBS = be shared translation localestub $(STDCPPLIBS)
LIBS += scintilla columnlistview tracker

LIBPATHS = $(shell findpaths -a $(platform) B_FIND_PATH_DEVELOP_LIB_DIRECTORY)
#$(info LIBPATHS="$(LIBPATHS)")

SYSTEM_INCLUDE_PATHS  = $(shell findpaths -e B_FIND_PATH_HEADERS_DIRECTORY private/interface)
SYSTEM_INCLUDE_PATHS +=	$(shell findpaths -e B_FIND_PATH_HEADERS_DIRECTORY private/shared)
SYSTEM_INCLUDE_PATHS +=	$(shell findpaths -a $(platform) -e B_FIND_PATH_HEADERS_DIRECTORY scintilla)

################################################################################
## clang++ headers hack
ifeq ($(platform), x86)
###### x86 clang++ build (mind scan-build too) #################################
ifneq ($(CXX), g++-x86)
SYSTEM_INCLUDE_PATHS += /boot/system/lib/x86/clang/5.0.0/include \
	/system/develop/tools/x86/lib/gcc/i586-pc-haiku/5.4.0/include/c++ \
	/system/develop/tools/x86/lib/gcc/i586-pc-haiku/5.4.0/include/c++/i586-pc-haiku
endif
else
######## x86_64 clang++ build (mind scan-build too) ############################
ifneq ($(CXX), g++)
SYSTEM_INCLUDE_PATHS += \
	/system/develop/tools/lib/gcc/x86_64-unknown-haiku/5.4.0/include/c++ \
	/system/develop/tools/lib/gcc/x86_64-unknown-haiku/5.4.0/include/c++/x86_64-unknown-haiku
endif
endif

OPTIMIZE := FULL

CFLAGS := -Wall -Werror

CXXFLAGS := -std=c++11

LOCALES := en it

DEBUGGER := TRUE


## Include the Makefile-Engine
DEVEL_DIRECTORY := \
	$(shell findpaths -r "makefile_engine" B_FIND_PATH_DEVELOP_DIRECTORY)
 include $(DEVEL_DIRECTORY)/etc/makefile-engine

## CXXFLAGS rule
$(OBJ_DIR)/%.o : %.cpp
	$(CXX) -c $< $(INCLUDES) $(CFLAGS) $(CXXFLAGS) -o "$@"

