### Generated by Winemaker 0.7.4


SRCDIR                = .
SUBDIRS               =
DLLS                  =
EXES                  = oxygenspread.exe



### Common settings

CEXTRA                = -mno-cygwin
CXXEXTRA              = -mno-cygwin
RCEXTRA               =
DEFINES               =
INCLUDE_PATH          = -I.
DLL_PATH              =
DLL_IMPORTS           =
LIBRARY_PATH          =
LIBRARIES             =


### oxygenspread.exe sources and settings

oxygenspread_exe_MODULE= oxygenspread.exe
oxygenspread_exe_C_SRCS= cellinfo.c \
			file_csv.c \
			file_kiss.c \
			history.c \
			layout.c \
			mymenu.c \
			oxygenspread.c \
			selection.c \
			stream.c \
			style.c \
			table.c \
			utility.c \
			view.c
oxygenspread_exe_CXX_SRCS=
oxygenspread_exe_RC_SRCS= En.rc \
			oxygenspread_res.rc
oxygenspread_exe_LDFLAGS= -mwindows \
			-mno-cygwin \
			-m32
oxygenspread_exe_DLL_PATH=
oxygenspread_exe_DLLS = odbc32 \
			ole32 \
			oleaut32 \
			winspool \
			odbccp32
oxygenspread_exe_LIBRARY_PATH=
oxygenspread_exe_LIBRARIES= uuid

oxygenspread_exe_OBJS = $(oxygenspread_exe_C_SRCS:.c=.o) \
			$(oxygenspread_exe_CXX_SRCS:.cpp=.o) \
			$(oxygenspread_exe_RC_SRCS:.rc=.res)



### Global source lists

C_SRCS                = $(oxygenspread_exe_C_SRCS)
CXX_SRCS              = $(oxygenspread_exe_CXX_SRCS)
RC_SRCS               = $(oxygenspread_exe_RC_SRCS)


### Tools

CC = winegcc
CXX = wineg++
RC = wrc


### Generic targets

all: $(SUBDIRS) $(DLLS:%=%.so) $(EXES:%=%.so)

### Build rules

.PHONY: all clean dummy

$(SUBDIRS): dummy
	@cd $@ && $(MAKE)

# Implicit rules

.SUFFIXES: .cpp .rc .res
DEFINCL = $(INCLUDE_PATH) $(DEFINES) $(OPTIONS)

.c.o:
	$(CC) -c $(CFLAGS) $(CEXTRA) $(DEFINCL) -o $@ $<

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(CXXEXTRA) $(DEFINCL) -o $@ $<

.cxx.o:
	$(CXX) -c $(CXXFLAGS) $(CXXEXTRA) $(DEFINCL) -o $@ $<

.rc.res:
	$(RC) $(RCFLAGS) $(RCEXTRA) $(DEFINCL) -fo$@ $<

# Rules for cleaning

CLEAN_FILES     = y.tab.c y.tab.h lex.yy.c core *.orig *.rej \
                  \\\#*\\\# *~ *% .\\\#*

clean:: $(SUBDIRS:%=%/__clean__) $(EXTRASUBDIRS:%=%/__clean__)
	$(RM) $(CLEAN_FILES) $(RC_SRCS:.rc=.res) $(C_SRCS:.c=.o) $(CXX_SRCS:.cpp=.o)
	$(RM) $(DLLS:%=%.so) $(EXES:%=%.so) $(EXES:%.exe=%)

$(SUBDIRS:%=%/__clean__): dummy
	cd `dirname $@` && $(MAKE) clean

$(EXTRASUBDIRS:%=%/__clean__): dummy
	-cd `dirname $@` && $(RM) $(CLEAN_FILES)

### Target specific build rules
DEFLIB = $(LIBRARY_PATH) $(LIBRARIES) $(DLL_PATH) $(DLL_IMPORTS:%=-l%)

$(oxygenspread_exe_MODULE).so: $(oxygenspread_exe_OBJS)
	$(CC) $(oxygenspread_exe_LDFLAGS) -o $@ $(oxygenspread_exe_OBJS) $(oxygenspread_exe_LIBRARY_PATH) $(DEFLIB) $(oxygenspread_exe_DLLS:%=-l%) $(oxygenspread_exe_LIBRARIES:%=-l%)


