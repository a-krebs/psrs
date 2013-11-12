OUT=		psrs
OBJS=		main.o
#TESTOBJS=	test.o $(SHAREDOBJS)

CFLAGS+=	-Wall

MPICC=		/home/aaron/bin/mpicc
MPIEXEC=	/home/aaron/bin/mpiexec

COMPILE=	$(MPICC) -c $< -o $@
LINK=		$(MPICC) -o $@ $+

all: CFLAGS+= -O2
all: $(OUT)

debug: CFLAGS+= -g -DDEBUG
debug: $(OUT)

#test: CFLAGS+= -g -DTEST

psrs: $(OBJS)
	$(LINK)

main.o: main.c
	$(COMPILE)

#test: $(TESTOBJS)
#	$(CC) -o test $(TESTOBJS) 

clean:
	-rm -f *.o $(OUT)

# useful utility rules
run4: $(OUT)
	$(MPIEXEC) -np 4 ./psrs
