all: lc3bsim2 A B

lc3bsim2: lc3bsim2.c
	gcc -ansi -Wall  lc3bsim2.c -lm -o lc3bsim2

A: lc3bsim2.c
	gcc -ansi -Wall  lc3bsim2.c -lm -o A

B: lc3bsim2.c
	gcc -ansi -Wall  lc3bsim2.c -lm -o B

gradeA: A
	./runA.sh

gradeB: B
	./runB.sh

grade: A B
	./run.sh



clean:
	rm -f A B lc3bsim2 test/lc3bsim2 test/dumpsim
