###########################################################################
#
# Define project informations
#
###########################################################################
#
MODULE_NAME  = trace
LIBRARY_NAME = trace.lib

include ./../../make/user.mk

#--------------------------------------------------------------------------
# Define place the project starts. In most cases it is current directory
#--------------------------------------------------------------------------
#
BASEDIR=.
CMPINVFILE=$(BASEDIR)/../../make/compiler.pl
LNKINVFILE=$(BASEDIR)/../../make/linker.pl
LIBINVFILE=$(BASEDIR)/../../make/librarian.pl

#--------------------------------------------------------------------------
# Define the main places used for module build. Use information obtained
# from BUILDTYPE variable
#--------------------------------------------------------------------------
#
SRCDIR     =$(BASEDIR)/src
INCDIR     =$(BASEDIR)/include
OBJDIR     =$(SRCDIR)/obj/$(OBJSUBDIR)
EXEDIR     =$(BASEDIR)/bin/$(EXESUBDIR)
LIBDIR     =$(BASEDIR)/lib/$(LIBSUBDIR)

#--------------------------------------------------------------------------
# The importand directiries are specified bellow.
# All external directories should be defined here. They will be used
# later for defining includes and libraries
#--------------------------------------------------------------------------
#
API_C_DIR  =$(BASEDIR)/../../becklibs/sc12_api
COMMON_DIR =$(BASEDIR)/../../common

#--------------------------------------------------------------------------
# The include files are placed in soooo many directories....
# Use them later for INCLUDES variable.
#--------------------------------------------------------------------------
#
INCLUDEDIR 	 =$(BASEDIR)/include
API_C_INCLUDEDIR =$(API_C_DIR)/include
COMMON_INCLUDE   =$(COMMON_DIR)/include

#--------------------------------------------------------------------------
# The library directories must be defined too. The library path can be
# used as library prefix, as well as search path for linker.
#--------------------------------------------------------------------------
#
API_C_LIBDIR=$(API_C_DIR)/lib/$(OBJSUBDIR)

COMP_INV_FILE=$(BASEDIR)/compiler.inv
LNK_INV_FILE=$(BASEDIR)/linker.inv
LIB_INV_FILE=$(BASEDIR)/library.inv

#--------------------------------------------------------------------------
# Include path is set of paths
#--------------------------------------------------------------------------
#
INCLUDES = -I$(BORLAND_INCLUDE) \
	   -I$(API_C_INCLUDEDIR) \
	   -I$(COMMON_INCLUDE) \
	   -I$(INCLUDEDIR)

###########################################################################
# The list of source files is defined here (objects list is automaticaly
# taken from source list). This way of files definition causes objects to
# be stored in source directory - it is acceptable
###########################################################################
#
# ----- SOURCES --------
LIBRARY_SRCS  = $(SRCDIR)/trace.c

# ----- OBJECTS --------
LIBRARY_OBJS = $(subst .c,.obj,$(subst $(SRCDIR),$(OBJDIR),$(LIBRARY_SRCS)))
TLIB_OBJS = $(subst /,\,$(subst .c,.obj,$(subst $(SRCDIR),+-$(OBJDIR),$(LIBRARY_SRCS))))

#--------------------------------------------------------------------------
# To build all executables and libraries execute this!
#--------------------------------------------------------------------------
#
all:     program

program: makeinv makedirs $(LIBDIR)/$(LIBRARY_NAME)
	 @echo "ALL DONE"

build: 	 clean all
	 @echo "BUILD DONE"

makeinv: 
	@perl $(CMPINVFILE) -n$(OBJDIR) $(CFLAGS) $(INCLUDES) 
	@perl $(LIBINVFILE) $(TLIB_OBJS)

makedirs: $(OBJDIR) $(LIBDIR)

$(OBJDIR):
	-$(MKDIRCMD) $(SRCDIR)/obj
	-$(MKDIRCMD) $(OBJDIR)

$(LIBDIR):
	-$(MKDIRCMD) $(BASEDIR)/lib
	-$(MKDIRCMD) $(LIBDIR)

#--------------------------------------------------------------------------
# To build libraries use commands as follows 
#--------------------------------------------------------------------------
# 
$(LIBDIR)/$(LIBRARY_NAME):$(LIBRARY_OBJS)
	@echo "Loading $@ ... "
	$(LIB) $(LIBFLAGS) $(subst /,\,$@) $(subst /,\,@$(LIB_INV_FILE))
	@echo "... done."

#--------------------------------------------------------------------------
# Buils object files using suffix definition
#--------------------------------------------------------------------------
#
$(OBJDIR)/%.obj : $(SRCDIR)/%.c
	@echo Compiling $< ........
	@$(CC) +$(COMP_INV_FILE) -c $<

#--------------------------------------------------------------------------
# Clean all 
#--------------------------------------------------------------------------

clean:
	$(RMDIRCMD) $(SRCDIR)/obj
	$(RMDIRCMD) $(BASEDIR)/lib
	$(RMCMD) $(COMP_INV_FILE)
	$(RMCMD) $(LIB_INV_FILE)

