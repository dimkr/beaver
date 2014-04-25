##
## Makefile for Beaver
## 
## Author<s>:   Marc Bevand (aka "After") <bevand_m@epita.fr>
## Last update: Mon Jul 15 22:29:51 CEST 2002
##

############## Public section. Modify according to your needs.  ###############

CC      = gcc
RM      = rm -f
CP      = cp

DESTDIR = /usr/local
IPATH   = 
LPATH   = 
OPTI    = -O3 -funroll-loops -fomit-frame-pointer #-mcpu=i686
DBUG    = #-ggdb #-pg
WARN	= #-W -Wall #-pedantic -ansi

### Unix version ###
CFLAGS  = ${OPTI} ${DBUG} ${WARN} ${IPATH} `gtk-config --cflags`
LDFLAGS = ${OPTI} ${DBUG} ${WARN} ${LPATH} `gtk-config --libs`
NAME    = beaver

### Windows version ###
#CFLAGS  = ${OPTI} ${DBUG} ${WARN} ${IPATH}\
#	   -fnative-struct -mwindows
#LDFLAGS = ${OPTI} ${DBUG} ${WARN} ${LPATH}\
#	   -lgdk-1.3 -lgtk-1.3 -lgmodule-1.3 -lglib-1.3 -lm
#NAME    = beaver.exe

###################### Private section. Do not modify  ########################

ifndef DBUG
	STRIP = strip --strip-all ${NAME}
endif

SRC	= main.c search.c tools.c languages.c editor.c prefs.c conf.c msgbar.c\
	  toolbar.c interface.c filesops.c completion.c undoredo.c
INC	= ${SRC:.c=.h} struct.h wordfile.h
OBJ	= ${SRC:.c=.o}

all	: ${OBJ}
	${CC} -o ${NAME} ${OBJ} ${LDFLAGS}
	@${STRIP}

.c.o	:
	${CC} ${CFLAGS} -c $< -o $@

${OBJ}	: ${INC}

install :
	install -d $(DESTDIR)/bin
	install beaver $(DESTDIR)/bin
	install -d $(DESTDIR)/share/beaver/bl
	$(CP) ../bl/* $(DESTDIR)/share/beaver/bl
	install -d $(DESTDIR)/share/pixmaps
	$(CP) ../pixmaps/beaver.png $(DESTDIR)/share/pixmaps
	gzip -c9 ../beaver.1x > ../beaver.1x.gz
	install -d $(DESTDIR)/man/man1
	$(CP) ../beaver.1x.gz $(DESTDIR)/man/man1
	@echo
	@echo "Files installed :"
	@echo "-----------------"
	@echo
	@echo "$(DESTDIR)/bin/beaver"
	@echo "$(DESTDIR)/share/beaver/bl/example1.bl"
	@echo "$(DESTDIR)/share/beaver/bl/example2.bl"
	@echo "$(DESTDIR)/share/beaver/bl/glib.bl"
	@echo "$(DESTDIR)/share/pixmaps/beaver.png"
	@echo "$(DESTDIR)/man/man1/beaver.1x.gz"
	@echo
	@echo ",----------------------------------."
	@echo "| Beaver succesfully installed ^_^ |"
	@echo "\`----------------------------------'"

.PHONY: clean re

clean	:
	${RM} *~ \#* .\#* *.core gmon.out ../beaver.1x.gz ${OBJ} ${NAME}

re	: clean all
