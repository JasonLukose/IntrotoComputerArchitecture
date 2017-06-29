# IntrotoComputerArchitecture

Computer Architecture, Concurrency and Energy
Labs are focused around teaching aspects of Computer Architecture using C language. 

The assembly language these labs are focused around is LC3-B. Byte addressable LC3.

## Lab 1 : Single Cycle Datapath Simulator

This lab contains the introductory Lab and all of its resources. Makefile and hex/asm files are provided for source code validation.
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



