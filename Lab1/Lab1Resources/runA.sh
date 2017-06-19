#!/bin/bash

cp A test/lc3bsim
cd test

# -----------------------------------------------
# -----------------------------------------------
# -----------------------------------------------
#  Part A: Testing single instruction at a time
# -----------------------------------------------
# -----------------------------------------------
# -----------------------------------------------

arr=(1 1 
2 1 
3 1 
4 1 
5 1 
6 1 
7 1 
8 1 
9 1 
10 1 
#11 1 # disabled due to TRAP already implemented
#12 1 # disabled due to TRAP already implemented
#13 1 # disabled due to TRAP already implemented
14 1 
15 1 
16 1 
17 1 
18 1 
19 1 
20 1 
21 1 
22 1 
23 1 
24 1 
25 1 
26 1 
27 1 
28 1 
29 1 
30 1 
#31 1 # disabled due to TRAP already implemented
32 2 
33 2 
34 2 
35 2 
36 2)
score=64
score_per=2
for ((i = 0; i < ${#arr[@]}; i = i + 2))
do
	printf "Running test ${arr[i]}.hex -- "
	./run ./lc3bsim state_data_in/${arr[i]}.hex ${arr[i+1]} > /dev/null 2>&1
	diff dumpsim state_data_out/${arr[i]}.dump > out
	if [ -s ./out ]
	then
		echo "failed"
		score=$(echo "scale=6; ${score}-${score_per}"|bc)
	else
		echo
	fi
done

echo "Your score is $score out of 64"
