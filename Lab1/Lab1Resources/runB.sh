#!/bin/bash



cp B test/lc3bsim
cd test

score=36

# -----------------------------------------------
# -----------------------------------------------
# -----------------------------------------------
# Part B: Testing functionality of multiple instructions
# -----------------------------------------------
# -----------------------------------------------
# -----------------------------------------------


arr=("add" 4	
"and" 23 
"br_noz" 27	
"jmp" 5	
"jsr" 8	
"jsrr" 9	
"ldb" 8	
"ldw" 5	
"lea" 2	
"lshf" 10	
"rshfa" 10	
"rshfl" 10	
"stb" 12	
"stw" 11	
"xor" 7	)

score_per=2.4

for ((i = 0; i < ${#arr[@]}; i = i + 2))
do
	printf "Running test ${arr[i]}.hex -- "
	./run2 ./lc3bsim data_in/${arr[i]}.hex ${arr[i+1]} > /dev/null 2>&1
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
