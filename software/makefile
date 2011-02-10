###########################################################################
#
# Define project informations
#
###########################################################################
#
MODULE_NAME  = edrive
PROGRAM_NAME = edrive.ihx

include ./make/user.mk

#--------------------------------------------------------------------------
# Define place the project starts. In most cases it is current directory
#--------------------------------------------------------------------------
#
BASEDIR=.
CMPINVFILE=$(BASEDIR)/make/compiler.pl
LNKINVFILE=$(BASEDIR)/make/linker.pl

LFLAGS = --stack-loc 0x81

#--------------------------------------------------------------------------
# Define the main places used for module build. Use information obtained
# from BUILDTYPE variable
#--------------------------------------------------------------------------
#
SRCDIR        =$(BASEDIR)/src
INCDIR        =$(BASEDIR)/include
OBJDIR        =$(SRCDIR)/obj
EXEDIR        =$(BASEDIR)/bin

#--------------------------------------------------------------------------
# The importand directiries are specified bellow.
# All external directories should be defined here. They will be used
# later for defining includes and libraries
#--------------------------------------------------------------------------
#
TRACE_DIR     	=$(BASEDIR)/libs/trace
LCD_DIR   	=$(BASEDIR)/libs/lcd

#--------------------------------------------------------------------------
# The include files are placed in soooo many directories....
# Use them later for INCLUDES variable.
#--------------------------------------------------------------------------
#
INCLUDEDIR   	=$(BASEDIR)/include
TRACE_INCLUDE   =$(API_C_DIR)/include
LCD_INCLUDE   	=$(STORAGE_DIR)/include

#--------------------------------------------------------------------------
# The library directories must be defined too. The library path can be
# used as library prefix, as well as search path for linker.
#--------------------------------------------------------------------------
#
TRACE_LIBDIR    =$(API_C_DIR)/lib/$(OBJSUBDIR)
LCD_LIBDIR   	=$(STORAGE_DIR)/lib/$(OBJSUBDIR)

COMP_INV_FILE =$(BASEDIR)/compiler.inv
LNK_INV_FILE  =$(BASEDIR)/linker.inv

#--------------------------------------------------------------------------
# Include path is set of paths
#--------------------------------------------------------------------------
#
INCLUDES = -I$(INCLUDEDIR)

#--------------------------------------------------------------------------
# If it is possible, then it is better to specify library as
# search path and the file name instead of full library names.
#--------------------------------------------------------------------------
#
LLIBS = 
#$(subst /,\,$(SYSTIME_LIB_DIR)/systime.lib+) \

#--------------------------------------------------------------------------
# Compiler standard libraries.
#
#--------------------------------------------------------------------------
#
STARTUP_OBJ =
STD_LIBS    =

###########################################################################
#
# The list of c source files, nedded to build executable file.
# Remember to put '+' sign in the end of the sequence if using Linker stage
#
###########################################################################
#
#
# ----- SOURCES -------
#

PROGRAM_SRCS =  	$(SRCDIR)/edrive.c \
		$(SRCDIR)/rtc.c \
		$(SRCDIR)/pull.c \
		$(SRCDIR)/flash.c \
		$(SRCDIR)/menu.c \
		$(SRCDIR)/lcd.c \
		$(SRCDIR)/keypad.c \
		$(SRCDIR)/pwm.c \
		$(SRCDIR)/pswd.c \
		$(SRCDIR)/salarm.c

#
# ----- OBJECTS --------
#

PROGRAM_OBJS =$(subst .c,.rel,$(subst $(SRCDIR),$(OBJDIR),$(PROGRAM_SRCS)))
LINKER_OBJS =$(subst /,\,$(subst .c,.rel,$(subst .c ,.rel$(COMMA) ,$(subst $(SRCDIR),$(OBJDIR),$(PROGRAM_SRCS)))))

#--------------------------------------------------------------------------
# Define list of modules directories
#--------------------------------------------------------------------------
#
MODULES = 
#	  $(BASEDIR)/../../becklibs/sc12_api \

#--------------------------------------------------------------------------
# To build all executables and libraries execute this!
#--------------------------------------------------------------------------
#
all:     program

link: $(EXEDIR)/$(PROGRAM_NAME) 

program: makeinv makedirs $(MODULES) $(EXEDIR)/$(PROGRAM_NAME) 
	 @echo "ALL DONE"

build: 	 clean all
	 @echo "BUILD DONE"

makeinv: 
	@perl $(CMPINVFILE) -n$(OBJDIR) $(CFLAGS) $(INCLUDES) 
	@perl $(LNKINVFILE) $(LFLAGS) $(LINKER_OBJS)

#	@perl $(LNKINVFILE) $(LFLAGS) $(LINKER_OBJS) $(subst /,\,$(EXEDIR)/$(PROGRAM_NAME)), $(STD_LIBS) $(LLIBS)

makedirs: $(OBJDIR) $(EXEDIR)

mv:
	$(MVCMD) $(OBJDIR)/$(PROGRAM_NAME) $(EXEDIR)/$(MODULE_NAME).hex
	@echo "... done."

$(OBJDIR):
	-$(MKDIRCMD) $(SRCDIR)/obj
	-$(MKDIRCMD) $(OBJDIR)

$(EXEDIR):
	-$(MKDIRCMD) $(BASEDIR)/bin
	-$(MKDIRCMD) $(EXEDIR)

$(MODULES) : NOTHING
	@echo ##############################################################
	@echo Entering subdirectory $@
	@echo Command $(COMMAND)
	@echo ##############################################################
	$(MAKE) -C $@ $(COMMAND)

#--------------------------------------------------------------------------
# To build exexutables file 
#--------------------------------------------------------------------------
# 

$(EXEDIR)/$(PROGRAM_NAME): $(PROGRAM_OBJS)
	@echo moving  ...
	$(MVCMD) $(SRCDIR)/*.rel $(OBJDIR)
	$(MVCMD) $(SRCDIR)/*.asm $(OBJDIR)
	$(MVCMD) $(SRCDIR)/*.lst $(OBJDIR)
	$(MVCMD) $(SRCDIR)/*.sym $(OBJDIR)
	@echo Loading $@ ...
	$(CC) $(LFLAGS) $(OBJDIR)/*.rel

	$(MVCMD) $(OBJDIR)/$(PROGRAM_NAME) $(EXEDIR)/$(MODULE_NAME).hex
	@echo "... done."

hex: 
	$(MVCMD) $(OBJDIR)/$(PROGRAM_NAME) $(EXEDIR)/$(MODULE_NAME).hex
	@echo "... done."

#	$(HEX_FORMAT) $(OBJDIR)/$(MODULE_NAME)

#--------------------------------------------------------------------------
# Buils object files using suffix definition
#--------------------------------------------------------------------------
#

$(PROGRAM_OBJS) : $(PROGRAM_SRCS)

$(OBJDIR)/%.rel : $(SRCDIR)/%.c
	@echo Compiling file $< ........
	@$(CC) -c --stack-auto $(INCLUDES) $<

#--------------------------------------------------------------------------
# Clean all 
#--------------------------------------------------------------------------

clean:
	$(RMDIRCMD) $(SRCDIR)/obj
	$(RMDIRCMD) $(BASEDIR)/bin
	$(RMCMD) $(COMP_INV_FILE)
	$(RMCMD) $(LNK_INV_FILE)

NOTHING:





