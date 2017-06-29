# IntrotoComputerArchitecture

Computer Architecture, Concurrency and Energy
Labs are focused around teaching aspects of Computer Architecture using C language. 

The assembly language these labs are focused around is LC3-B. Byte addressable LC3.

The base code for these labs were provided by the professor.

## Lab 1 : Single Cycle Datapath Simulator

This directory contains the introductory Lab and all of its resources. Makefile and hex/asm files are provided for source code validation.
Single Cycle Datapaths revolve around the idea that it only takes one single cycle for any instruction to be completed.
Even though this may sound ideal, some instructions take longer than others to complete (such as ADD vs LDB), resulting in quick
operations finishing quickly and leaving the system idle.

Example of Code that was written: 

```C
/*
    ADD INSTRUCTION: TAKES SR1 + SR2 of current latches and stores it into next latch's DR
    Else: Use the immediate value
    Also sets condition code based on DR Value
*/
 void execute_ADD(){
    if (INST.A == 0) {
        NEXT_LATCHES.REGS[INST.DR] = Low16bits(CURRENT_LATCHES.REGS[INST.SR1] + CURRENT_LATCHES.REGS[INST.SR2]); 
    } else {
         NEXT_LATCHES.REGS[INST.DR] = Low16bits (CURRENT_LATCHES.REGS[INST.SR1] + SEXT(INST.imm5,5));
        }
    setcc(NEXT_LATCHES.REGS[INST.DR]);  
 }

/*
    BR: Checks the n,z,p and & each seperately then finally OR'd together
    address is shifted by an offset
*/

 void execute_BR(){
    if ((INST.n & CURRENT_LATCHES.N) |(INST.p & CURRENT_LATCHES.P) | (INST.z & CURRENT_LATCHES.Z)){
        int addressBR = LSHF(SEXT(INST.PCoffset9,9),1);
        NEXT_LATCHES.PC = NEXT_LATCHES.PC + addressBR; 
    }  
 }

/*
```

## Lab2 : Multicycle Datapath Simulator

The Lab2 directory contains all the resources for the Multicycle datapath simulator, written in C. The base code contained information regarding the latches, registers and flow of the cycle's stages (such as, eval_micro_sequencer). We were required to simulate what occurs in each stage.

This type of datapath allows instructions to finish based on how much work is actually done. One example is ADD vs LDB, the ADD Instruction does not require Memory access so the memory does not have to be cycled for the instruction to finish. However, the LDB is limited by the memory access function and will wait for a READY from Memory before the rest of the stages are completed. This shows significant improvement in efficency from the Single Cycle Datapath but still is limited by the fact that instructions need to wait for the earlier instruction to finish before execution. 

Example code from the cycle_memory stage

```C
 /* 
   * This function emulates memory and the WE logic. 
   * Keep track of which cycle of MEMEN we are dealing with.  
   * If fourth, we need to latch Ready bit at the end of 
   * cycle to prepare microsequencer for the fifth cycle.  
   */
void cycle_memory() {
    if (GetMIO_EN(CURRENT_LATCHES.MICROINSTRUCTION) == 1) {
        ready_mem = ready_mem + 1;
    /* if MEM Enabled, add 1 and check if ready */    
        if (ready_mem == 4) {
            NEXT_LATCHES.READY = 1; 
        }
    /*After it is ready (on 5th cycle), READ/WRITE */
        if (ready_mem == 5) {
            if (!GetR_W(CURRENT_LATCHES.MICROINSTRUCTION)) { /* Reading from Memory */
                    /* This is for reading the whole word from memory */
                    NEXT_LATCHES.MDR = read_word(Low16bits(CURRENT_LATCHES.MAR));
            } else {
                /* This is for writing to memory */
                int MAR0 = get_bits(CURRENT_LATCHES.MAR, 0, 0);
                int dSize = GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION);
                
                /* ASSERT BOTH WE0 and WE1: DATA_SIZE = 1 and MAR[0] = 0 */
                if (dSize && !MAR0) {
                    /* BYTE STORE and MAR[0] = 1: MDR[15:8] is written to MEMORY */
	 	   MEMORY[CURRENT_LATCHES.MAR >> 1][1] = get_bits(CURRENT_LATCHES.MDR,15,8);
                    MEMORY[CURRENT_LATCHES.MAR >> 1][0] = get_bits(CURRENT_LATCHES.MDR,7,0);
                } else if (!dSize && MAR0) {
                    /* BYTE STORE and MAR[0] = 1: MDR[15:8] is written to MEMORY */
	 	   MEMORY[CURRENT_LATCHES.MAR >> 1][1] = get_bits(CURRENT_LATCHES.MDR,15,8);
                } else if (!dSize && !MAR0) {
                    /* BYTE STORE and MAR[0] = 0; MDR[7:0] is written to MEMORY */
                    MEMORY[CURRENT_LATCHES.MAR >> 1][0] = get_bits(CURRENT_LATCHES.MDR,7,0);
		}
            }

            /* AFTER READ/WRITE on 5th CYCLE, READY = 0 and READY_MEM back to 0 */ 
            NEXT_LATCHES.READY = 0;
            ready_mem = 0;
        } 
    } 
}
```


