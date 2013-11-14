OUT=		psrs
OBJS=		main.o args.o phases.o
#TESTOBJS=	test.o $(SHAREDOBJS)

CFLAGS+=	-Wall

MPICC=		/home/aaron/bin/mpicc
MPIEXEC=	/home/aaron/bin/mpiexec

COMPILE=	$(MPICC) $(CFLAGS) -c $< -o $@
LINK=		$(MPICC) $(CFLAGS) -o $@ $+

all: CFLAGS+= -O2
all: $(OUT)

debug: CFLAGS+= -g -DDEBUG
debug: $(OUT)

#test: CFLAGS+= -g -DTEST

psrs: $(OBJS)
	$(LINK)

main.o: main.c
	$(COMPILE)

args.o: args.c args.h
	$(COMPILE)

phases.o: phases.c phases.h
	$(COMPILE)

#test: $(TESTOBJS)
#	$(CC) -o test $(TESTOBJS) 

clean:
	-rm -f core *.o $(OUT)

# useful utility rules
run4: all
	@$(MPIEXEC) -np 3 ./psrs -n 36
