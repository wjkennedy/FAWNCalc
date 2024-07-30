#You may wish to add -DNO_UNISTD and -DNO_SYSTEM to CFLAGS if
#your system lacks the /usr/include/unistd.h header file
#or the system() function, respectively. Non-unix systems
#will also definitely have different linker syntax.

prefix := ./
exec_prefix := ${prefix}/cgi-bin
tmp_html := ${exec_prefix}/TMP

CFLAGS=-Wall -O2
CC=gcc
AR=ar
LIBS=-L./ -lcgic -lcrypt

BDR_CFILES := rules.c parts.c eval.c html.c plot.c fnntab.c timer.c \
		 edit.c login.c save.c util.c email.c  ${LIBOBJDIR}mergesort$U.o
BDR_HFILES := config.h email.h rules.h util.h
BDR_OFILES := $(BDR_CFILES:.c=.o)

all: libcgic.a fawncalc.cgi

libcgic.a: cgic.o cgic.h rules.h
	rm -f libcgic.a
	$(AR) rc libcgic.a cgic.o

cgictest: cgictest.o libcgic.h
	${CC} ${CFLAGS} cgictest.o -o cgictest ${LIBS}

clean:
	rm -f *.o *.a cgictest

etags:
	etags ${BDR_CFILES} rules.h cgic.c cgic.h

oldrules.cgi: oldrules.o libcgic.a
	${CC} ${CFLAGS} oldrules.o -o oldrules.cgi ${LIBS}
	chmod +s oldrules.cgi

rules.o: rules.c rules.h cgic.h
	${CC} ${CFLAGS} rules.c -c

parts.o: parts.c rules.h cgic.h
	${CC} ${CFLAGS} parts.c -c

eval.o: eval.c rules.h cgic.h
	${CC} ${CFLAGS} eval.c -c 

html.o: html.c rules.h cgic.h
	${CC} ${CFLAGS} html.c -c 

edit.o: edit.c rules.h cgic.h
	${CC} ${CFLAGS} edit.c -c

login.o: login.c rules.h cgic.h util.h email.h
	${CC} ${CFLAGS} login.c -c

save.o: save.c rules.h cgic.h util.h email.h
	${CC} ${CFLAGS} save.c -c

util.o: util.c rules.h cgic.h util.h
	${CC} ${CFLAGS} util.c -c

email.o: email.c rules.h cgic.h email.h
	${CC} ${CFLAGS} email.c -c

plot.o: plot.c rules.h cgic.h config.h
	${CC} ${CFLAGS} plot.c -c 

fnntab.o: fnntab.c
	${CC} ${CFLAGS} fnntab.c -c

timer.o: timer.c rules.h
	${CC} ${CFLAGS} timer.c -c 

fawncalc.cgi: ${BDR_OFILES} libcgic.a
	${CC} ${CFLAGS} ${BDR_OFILES} -o fawncalc.cgi ${LIBS}

tags: TAGS
TAGS: ${BDR_CFILES} ${BDR_HFILES}
	etags ${BDR_CFILES} ${BDR_HFILES}

install: all
	#	chmod +s fawncalc.cgi
	mkdir -p ${tmp_html}
	cp -a fawncalc.cgi ${exec_prefix}/fawncalc.cgi
	@if [ ! -f ${exec_prefix}/cdrpasswd ] ; then \
		echo "nobody@bogusdomain:0:0:*" > ${exec_prefix}/cdrpasswd ; fi
	@if [ ! -f ${exec_prefix}/db0.cgtxt ]; then \
		cp -a db0.cgtxt ${exec_prefix}/db0.cgtxt ; fi
