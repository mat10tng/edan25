CC		= gcc
CFLAGS		= -O3 -maltivec -fopenmp -std=c11 -m64 -pthread -D_GNU_SOURCE -DPPC64
LDFLAGS		= -fopenmp -static -L. -lf2c -lm
OBJS		= swim.o timebase.o tbr.o main.o
OUT		= a.out

execute: clean compile
	./a.out < input > output
	diff output correct
	cat score

compile: $(OBJS)	
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $(OUT)

tbr.o:
	$(CC) -m64 tbr.s -c

clean:
	rm -f *.o a.out score SWIM7 output
