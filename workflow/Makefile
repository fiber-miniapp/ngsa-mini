PROG=		workflow
OBJS=		workflow.o

.SUFFIXES:.c .o

.c.o:
		$(CC) -c $(CFLAGS) $(DFLAGS) $< -o $@

all: $(PROG)

workflow: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

clean:
	rm -f $(PROG) *.o *~
