PROG=		splitSam2Contig2
OBJS=		splitSam2Contig2.o textin.o

.SUFFIXES:.c .o .cpp

.cpp.o:
		$(CXX) -c $(CFLAGS) $(DFLAGS) $< -o $@

all:$(PROG)

splitSam2Contig2: $(OBJS)
	$(CXX) $(CFLAGS) -o $@ $(OBJS)

splitSam2Contig2.o: my_types.h textin.h usafe.h
textin.o: textin.h usafe.h

clean:
	rm -f $(PROG) *.o *~
