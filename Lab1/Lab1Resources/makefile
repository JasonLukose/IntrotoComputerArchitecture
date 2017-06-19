all: lc3bsim A B

lc3bsim: lc3bsim1.c
	gcc -ansi -Wall  lc3bsim1.c -lm -o lc3bsim

A: lc3bsim1.c
	gcc -ansi -Wall -DINIT_REGS=1 lc3bsim1.c -lm -o A

B: lc3bsim1.c
	gcc -ansi -Wall  lc3bsim1.c -lm -o B

gradeA: A
	./runA.sh

gradeB: B
	./runB.sh

grade: A B
	./run.sh



clean:
	rm -f A B lc3bsim test/lc3bsim test/dumpsim
