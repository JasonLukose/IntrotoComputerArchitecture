#!/bin/bash

cp B test/lc3bsim2
cd test

score=36

# -----------------------------------------------
# -----------------------------------------------
# -----------------------------------------------
# Part B: Testing functionality of multiple instructions
# -----------------------------------------------
# -----------------------------------------------
# -----------------------------------------------


arr=("add" 42	
"and" 213 
"br" 234	
"jmp" 51	
"jsr" 79	
"jsrr" 89	
"ldb" 114	
"ldw" 69	
"lea" 24	
"lshf" 96	
"rshfa" 102	
"rshfl" 102	
"stb" 132	
"stw" 123	
"xor" 69	)

score_per=2.4

for ((i = 0; i < ${#arr[@]}; i = i + 2))
do
	printf "Running test ${arr[i]}.hex -- "
	./run2 ./lc3bsim2 ../ucode data_in/${arr[i]}.hex ${arr[i+1]} > /dev/null 2>&1
	diff dumpsim data_out/${arr[i]}.dump > out
	if [ -s ./out ]
	then
		echo "failed"
		score=$(echo "scale=6; ${score}-${score_per}"|bc)
	else
		echo
	fi
done

echo "Your score is $score out of 36"
