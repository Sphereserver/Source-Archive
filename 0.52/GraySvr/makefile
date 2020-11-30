# Generic makefile

OPT 	=

#WARN	= -Wall -W
WARN	= -w

EXE	= graysvr

CC	= g++

NORTII	= -fno-rtti
NOEX	= -fno-exceptions
EX	= -fexceptions
STRICT  = -mstrict-align

DEBUG	= -g
PROF	= -pg
PIPE	= -pipe

SRC	:= 	caccount.cpp \
		cbacktask.cpp \
		cbase.cpp \
		cchar.cpp \
		ccharbase.cpp \
		ccharfight.cpp \
		ccharnpc.cpp \
		cchat.cpp \
		cclient.cpp \
		cclientlog.cpp \
		cclientmsg.cpp \
		ccontain.cpp \
		citem.cpp \
		citembase.cpp \
		citemsp.cpp \
		citemstone.cpp \
		cmail.cpp \
		cserver.cpp \
		cworld.cpp \
		graysvr.cpp

COMMONSRC	:=	../common/carray.cpp \
			../common/ccrypt.cpp \
			../common/cexpression.cpp \
			../common/cfile.cpp \
			../common/cgrayinst.cpp \
			../common/cgraymap.cpp \
			../common/cregion.cpp \
			../common/cscript.cpp \
			../common/cstring.cpp 

INCLUDE	= -I/usr/ea/h -I../common

LIBS = -L/usr/lib

DEFINES = -DGRAY_SVR

SPECIAL = $(NORTTI) $(EX) $(STRICT)
STATIC	= -static
#O_FLAGS	= $(WARN) $(PIPE) $(PROF) $(STATIC) $(SPECIAL)
O_FLAGS	= $(WARN) $(PIPE) $(STATIC) 

C_FLAGS	= $(OPT) $(INCLUDE) $(DEFINES)

.cpp.o:
	@echo " Compiling $<"
	@ $(CC) -c $(O_FLAGS) $(C_FLAGS) $ < $(INCLUDE)

.PHONY:	all clean tidy

all:	$(EXE)

clean:	tidy
	rm -f *.o ../common/*.o $(EXE)
	$(MAKE)

.depend:
	gcc -MM $(INCLUDE) $(SRC) $(COMMONSRC) > .depend
	perl -pi -e 's/([^.]+)\.o/o\/\1.o/g' .depend

tidy:
	rm -f *~ *orig *bak *rej ../common/*~ ../common/*orig ../common*bak

tags:	$(SRC) $(COMMONSRC)
	ctags $(SRC) $(COMMONSRC)

cmn:	$(COMMONSRC:.cpp=.o)

gray:	$(SRC:.cpp=.o)

commonlib:	$(COMMONSRC:.cpp=.o)
	ar r $@ $?

$(EXE): commonlib gray
	g++ $(O_FLAGS) $(LIBS) -o $(EXE) *.o commonlib

version.h:	VERSION
	echo "#define VERSION 'head -l VERSION'" > version.h
	echo "#define COMPILED_BY \"$(COMPILED_BY)\"" >> version.h

%.o:	%.cpp
	@echo " Compiling $<"
	@$(CC) -c $(O_FLAGS) $(C_FLAGS) $< -o $@


