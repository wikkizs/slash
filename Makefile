CC = gcc
CFLAGS = -Wall -g -std=c11
LDLIBS = -lreadline
ALL = slash
all : $(ALL)
# test : test.o cd.o pwd.o
 slash : cd.o pwd.o exit.o
 cd.o : cd.c cd.h
 exit.o : exit.c exit.h
 pwd.o : pwd.c pwd.h
clean :
	rm -rf *~
cleanall : clean
	rm -rf slash *.o

