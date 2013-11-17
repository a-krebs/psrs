OUT=		psrs qsort
PSRS_OBJS=	psrs.o args.o phases.o
QSORT_OBJS=	qsort.o args.o tod.o

CFLAGS+=	-Wall

MPICC=		/home/aaron/bin/mpicc
MPIEXEC=	/home/aaron/bin/mpiexec

MPI_COMPILE=	$(MPICC) $(CFLAGS) -c $< -o $@
MPI_LINK=	$(MPICC) $(CFLAGS) -o $@ $+
COMPILE=	$(CC) $(CFLAGS) -c $< -o $@
LINK=		$(CC) $(CFLAGS) -o $@ $+

all: CFLAGS+= -O2
all: $(OUT)

debug: CFLAGS+= -g -DDEBUG -DGATHERFINAL
debug: $(OUT)

verify: CFLAGS+= -O2 -DGATHERFINAL
verify: $(OUT)

qsort: $(QSORT_OBJS)
	$(LINK)

qsort.o: qsort.c
	$(COMPILE)

psrs: $(PSRS_OBJS)
	$(MPI_LINK)

psrs.o: psrs.c
	$(MPI_COMPILE)

mpi_args.o: args.c args.h
	$(MPI_COMPILE)

args.o: args.c args.h
	$(COMPILE)

phases.o: phases.c phases.h
	$(MPI_COMPILE)

tod.o: tod.c tod.h
	$(COMPILE)

experiments.sh: generate_tests.py
	python generate_tests.py > experiments.sh
	chmod +x experiments.sh

clean:
	-rm -f core *.o $(OUT)

# useful utility rules
run6: all
	@$(MPIEXEC) -np 6 ./psrs -n 42

run_prep: clean all experiments.sh
