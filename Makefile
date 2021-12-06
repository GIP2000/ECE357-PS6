all: tas.o spinlock.o P2A.o sem.o P2C.o P2C.out
.PHONY: clean

tas.o: include/tas.h src/tas.S
	gcc -c src/tas.S

spinlock.o: src/spinlock.c include/spinlock.h include/tas.h
	gcc -c src/spinlock.c

P2A.o: src/P2A.c include/spinlock.h include/tas.h
	gcc -c src/P2A.c

P2A.out: P2A.o spinlock.o tas.o	
	gcc -o P2A.out P2A.o spinlock.o tas.o

sem.o: src/sem.c include/sem.h include/tas.h include/spinlock.h
	gcc -c src/sem.c

P2C.o: src/P2C.c include/sem.h include/tas.h include/spinlock.h 
	gcc -c src/P2C.c

P2C.out: P2C.o sem.o tas.o spinlock.o 
	gcc -g -o P2C.out P2C.o  sem.o tas.o spinlock.o
clean:
	rm -f *.o
	rm -f *.out