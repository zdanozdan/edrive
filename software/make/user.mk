#--------------------------------------------------------------------------
# The suffixes defined in Makefile 
#--------------------------------------------------------------------------
#
.SUFFIXES: .cpp .c .cxx

###########################################################################
#
# Only these system commands will be used
#
###########################################################################
#
#CC	 = c51
CC	 = sdcc
LINK	 = l51
LIB	 = tlib
PWDCMD   = pwd
RMCMD    = rm -f
RMDIRCMD = rm -rf
MKDIRCMD = mkdir.exe
MKDEPCMD = perl -S  mkdep.pl
MVCMD	 = mv -f
CPCMD    = cp
JAVA 	 = javac
JAVAAR   = jar
JAVADOC  = javadoc
BISON    = bison -d
LEX      = flex
KEYTOOL  = keytool
JAVASIGNER = jarsigner
HEX_FORMAT = oh51
COMMA=,
SPACE= 

ifneq "$(BUILDTYPE)" ""
BUILDMODE=$(BUILDTYPE)
else
BUILDMODE=rel
endif

ifneq "$(BUILDTYPE)" "dbg"
	ifneq "$(BUILDTYPE)" "rel"
		@echo "Illegal value 'BUILDTYPE=$(BUILDTYPE)'. Use 'dbg' or 'rel'. Building for default 'rel'"
		BUILDMODE=rel
	endif
endif

OBJSUBDIR =$(BUILDMODE)
LIBSUBDIR =$(BUILDMODE)
EXESUBDIR =$(BUILDMODE)


#--------------------------------------------------------------------------
#
# The important directories are specified bellow.
# 
#--------------------------------------------------------------------------
#
KEIL_DIR      	=c:\c51v4
KEIL_INCLUDE 	=$(BORLAND_DIR)\inc
KEIL_LIB_DIR 	=$(BORLAND_DIR)\lib

#--------------------------------------------------------------------------
#
# Linker is not worse then compilers - it keeps debug and release options
#
#--------------------------------------------------------------------------
#
ifeq "$(BUILDMODE)" "dbg"
LFLAGS =
CFLAGS = DEBUG
else
LFLAGS =
CFLAGS = DEBUG
endif

#--------------------------------------------------------------------------
#
# Librarian flags
#
#--------------------------------------------------------------------------
#
LIBFLAGS  =

#--------------------------------------------------------------------------
# The libraries will be used for linking. If it is necessary there can
# be defined many library sets, but linker will do a good job for us
# removin unnecessary symbols so it is easier to specify all libraries in
# one place.
#--------------------------------------------------------------------------
#
LIBSEARCHPATH=/L$(subst /,\,$(KEIL_LIB_DIR))

#--------------------------------------------------------------------------
#
# Other environment settings needed for compilation and linking.
#
#--------------------------------------------------------------------------
BISON_SIMPLE=C:\rktools\bin\bison.simple
