##############
#!/bin/bash

# need to add the initialize part here.
#!/bin/bash

# need to add the initialize part here.
# rm -f A
cp lc3bsim2.c test/lc3bsim2.mod.c
cd test

sed -i "/init_[Cc][Oo][Nn][Tt][Rr][Oo][Ll]_[Ss][Tt][Oo][Rr][Ee](ucode_filename)/a\
	 CURRENT_LATCHES.REGS\[0\] = 0;\n\
	 CURRENT_LATCHES.REGS\[1\] = 1;\n\
	 CURRENT_LATCHES.REGS\[2\] = 2;\n\
	 CURRENT_LATCHES.REGS\[3\] = 3;\n\
	 CURRENT_LATCHES.REGS\[4\] = 4;\n\
	 CURRENT_LATCHES.REGS\[5\] = 5;\n\
	 CURRENT_LATCHES.REGS\[6\] = 0x1236;\n\
	 CURRENT_LATCHES.REGS\[7\] = 0xABCD;" lc3bsim2.mod.c

gcc -ansi -Wall lc3bsim2.mod.c -lm -o lc3bsim2 > /dev/null 2>&1

# cp A test/lc3bsim2
# cd test

# -----------------------------------------------
# -----------------------------------------------
# -----------------------------------------------
#  Part A: Testing single instruction at a time
# -----------------------------------------------
# -----------------------------------------------
# -----------------------------------------------

arr=(1 1 
2 6 
3 7 
4 8 
5 9 
"6ccn" 9 
"7imm" 9 
"8ccp" 9 
9 9 
10 14 
11 15 
12 9 
13 10 
14 15 
15 9 
16 10 
17 15 
18 9 
19 10 
20 15)
score=100
score_per=3.2
for ((i = 0; i < ${#arr[@]}; i = i + 2))
do
	printf "Running test ${arr[i]}.hex -- "
	./run ./lc3bsim2 ../ucode state_data_in/${arr[i]}.hex ${arr[i+1]}> /dev/null 2>&1
	diff dumpsim state_data_out/${arr[i]}.dump > out
	if [ -s ./out ]
	then
		echo "failed"
		score=$(echo "scale=6; ${score}-${score_per}"|bc)
	else
		echo
	fi
done


cd ..
cp B test/lc3bsim2
cd test



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

echo "Your score is $score out of 100"
