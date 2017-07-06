/**************************************************************/
/*                                                             */
/* LC-3b Simulator (Adapted from Prof. Yale Patt at UT Austin) */
/*                                                             */
/***************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/***************************************************************/
/*                                                             */
/* Files:  ucode        Microprogram file                      */
/*         isaprogram   LC-3b machine language program file    */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void eval_micro_sequencer();
void cycle_memory();
void eval_bus_drivers();
void drive_bus();
void latch_datapath_values();

/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE  1

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)

/***************************************************************/
/* Definition of the control store layout.                     */
/***************************************************************/
#define CONTROL_STORE_ROWS 64
#define INITIAL_STATE_NUMBER 18

/***************************************************************/
/* Definition of bit order in control store word.              */
/***************************************************************/
enum CS_BITS {                                                  
    IRD,
    COND1, COND0,
    J5, J4, J3, J2, J1, J0,
    LD_MAR,
    LD_MDR,
    LD_IR,
    LD_BEN,
    LD_REG,
    LD_CC,
    LD_PC,
    GATE_PC,
    GATE_MDR,
    GATE_ALU,
    GATE_MARMUX,
    GATE_SHF,
    PCMUX1, PCMUX0,
    DRMUX,
    SR1MUX,
    ADDR1MUX,
    ADDR2MUX1, ADDR2MUX0,
    MARMUX,
    ALUK1, ALUK0,
    MIO_EN,
    R_W,
    DATA_SIZE,
    LSHF1,
    CONTROL_STORE_BITS
} CS_BITS;

/***************************************************************/
/* Functions to get at the control bits.                       */
/***************************************************************/
int GetIRD(int *x)           { return(x[IRD]); }
int GetCOND(int *x)          { return((x[COND1] << 1) + x[COND0]); }
int GetJ(int *x)             { return((x[J5] << 5) + (x[J4] << 4) +
				      (x[J3] << 3) + (x[J2] << 2) +
				      (x[J1] << 1) + x[J0]); }
int GetLD_MAR(int *x)        { return(x[LD_MAR]); }
int GetLD_MDR(int *x)        { return(x[LD_MDR]); }
int GetLD_IR(int *x)         { return(x[LD_IR]); }
int GetLD_BEN(int *x)        { return(x[LD_BEN]); }
int GetLD_REG(int *x)        { return(x[LD_REG]); }
int GetLD_CC(int *x)         { return(x[LD_CC]); }
int GetLD_PC(int *x)         { return(x[LD_PC]); }
int GetGATE_PC(int *x)       { return(x[GATE_PC]); }
int GetGATE_MDR(int *x)      { return(x[GATE_MDR]); }
int GetGATE_ALU(int *x)      { return(x[GATE_ALU]); }
int GetGATE_MARMUX(int *x)   { return(x[GATE_MARMUX]); }
int GetGATE_SHF(int *x)      { return(x[GATE_SHF]); }
int GetPCMUX(int *x)         { return((x[PCMUX1] << 1) + x[PCMUX0]); }
int GetDRMUX(int *x)         { return(x[DRMUX]); }
int GetSR1MUX(int *x)        { return(x[SR1MUX]); }
int GetADDR1MUX(int *x)      { return(x[ADDR1MUX]); }
int GetADDR2MUX(int *x)      { return((x[ADDR2MUX1] << 1) + x[ADDR2MUX0]); }
int GetMARMUX(int *x)        { return(x[MARMUX]); }
int GetALUK(int *x)          { return((x[ALUK1] << 1) + x[ALUK0]); }
int GetMIO_EN(int *x)        { return(x[MIO_EN]); }
int GetR_W(int *x)           { return(x[R_W]); }
int GetDATA_SIZE(int *x)     { return(x[DATA_SIZE]); } 
int GetLSHF1(int *x)         { return(x[LSHF1]); }

/***************************************************************/
/* The control store rom.                                      */
/***************************************************************/
int CONTROL_STORE[CONTROL_STORE_ROWS][CONTROL_STORE_BITS];

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A 
   There are two write enable signals, one for each byte. WE0 is used for 
   the least significant byte of a word. WE1 is used for the most significant 
   byte of a word. */

#define WORDS_IN_MEM    0x08000 
#define MEM_CYCLES      5
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */
int BUS;	/* value of the bus */

typedef struct System_Latches_Struct{

int PC,		/* program counter */
    MDR,	/* memory data register */
    MAR,	/* memory address register */
    IR,		/* instruction register */
    N,		/* n condition bit */
    Z,		/* z condition bit */
    P,		/* p condition bit */
    BEN;        /* ben register */

int READY;	/* ready bit */
  /* The ready bit is also latched as you dont want the memory system to assert it 
     at a bad point in the cycle*/

int REGS[LC_3b_REGS]; /* register file. */

int MICROINSTRUCTION[CONTROL_STORE_BITS]; /* The microintruction */

int STATE_NUMBER; /* Current State Number - Provided for debugging */ 
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int CYCLE_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands.                   */
/*                                                             */
/***************************************************************/
void help() {                                                    
    printf("----------------LC-3bSIM Help-------------------------\n");
    printf("go               -  run program to completion       \n");
    printf("run n            -  execute program for n cycles    \n");
    printf("mdump low high   -  dump memory from low to high    \n");
    printf("rdump            -  dump the register & bus values  \n");
    printf("?                -  display this help menu          \n");
    printf("quit             -  exit the program                \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {                                                

  eval_micro_sequencer();   
  cycle_memory();
  eval_bus_drivers();
  drive_bus();
  latch_datapath_values();

  CURRENT_LATCHES = NEXT_LATCHES;

  CYCLE_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles.                 */
/*                                                             */
/***************************************************************/
void run(int num_cycles) {                                      
    int i;

    if (RUN_BIT == FALSE) {
	printf("Can't simulate, Simulator is halted\n\n");
	return;
    }

    printf("Simulating for %d cycles...\n\n", num_cycles);
    for (i = 0; i < num_cycles; i++) {
	if (CURRENT_LATCHES.PC == 0x0000) {
	    RUN_BIT = FALSE;
	    printf("Simulator halted\n\n");
	    break;
	}
	cycle();
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : go                                              */
/*                                                             */
/* Purpose   : Simulate the LC-3b until HALTed.                 */
/*                                                             */
/***************************************************************/
void go() {                                                     
    if (RUN_BIT == FALSE) {
	printf("Can't simulate, Simulator is halted\n\n");
	return;
    }

    printf("Simulating...\n\n");
    while (CURRENT_LATCHES.PC != 0x0000)
	cycle();
    RUN_BIT = FALSE;
    printf("Simulator halted\n\n");
}

/***************************************************************/ 
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a word-aligned region of memory to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void mdump(FILE * dumpsim_file, int start, int stop) {          
    int address; /* this is a byte address */

    printf("\nMemory content [0x%04x..0x%04x] :\n", start, stop);
    printf("-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
	printf("  0x%04x (%d) : 0x%02x%02x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    printf("\n");

    /* dump the memory contents into the dumpsim file */
    fprintf(dumpsim_file, "\nMemory content [0x%04x..0x%04x] :\n", start, stop);
    fprintf(dumpsim_file, "-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
	fprintf(dumpsim_file, " 0x%04x (%d) : 0x%02x%02x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    fprintf(dumpsim_file, "\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current register and bus values to the     */   
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump(FILE * dumpsim_file) {                               
    int k; 

    printf("\nCurrent register/bus values :\n");
    printf("-------------------------------------\n");
    printf("Cycle Count  : %d\n", CYCLE_COUNT);
    printf("PC           : 0x%04x\n", CURRENT_LATCHES.PC);
    printf("IR           : 0x%04x\n", CURRENT_LATCHES.IR);
    printf("STATE_NUMBER : 0x%04x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    printf("BUS          : 0x%04x\n", BUS);
    printf("MDR          : 0x%04x\n", CURRENT_LATCHES.MDR);
    printf("MAR          : 0x%04x\n", CURRENT_LATCHES.MAR);
    printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    printf("Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
	printf("%d: 0x%04x\n", k, CURRENT_LATCHES.REGS[k]);
    printf("\n");

    /* dump the state information into the dumpsim file */
    fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
    fprintf(dumpsim_file, "-------------------------------------\n");
    fprintf(dumpsim_file, "Cycle Count  : %d\n", CYCLE_COUNT);
    fprintf(dumpsim_file, "PC           : 0x%04x\n", CURRENT_LATCHES.PC);
    fprintf(dumpsim_file, "IR           : 0x%04x\n", CURRENT_LATCHES.IR);
    fprintf(dumpsim_file, "STATE_NUMBER : 0x%04x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    fprintf(dumpsim_file, "BUS          : 0x%04x\n", BUS);
    fprintf(dumpsim_file, "MDR          : 0x%04x\n", CURRENT_LATCHES.MDR);
    fprintf(dumpsim_file, "MAR          : 0x%04x\n", CURRENT_LATCHES.MAR);
    fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    fprintf(dumpsim_file, "Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
	fprintf(dumpsim_file, "%d: 0x%04x\n", k, CURRENT_LATCHES.REGS[k]);
    fprintf(dumpsim_file, "\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : get_command                                     */
/*                                                             */
/* Purpose   : Read a command from standard input.             */  
/*                                                             */
/***************************************************************/
void get_command(FILE * dumpsim_file) {                         
    char buffer[20];
    int start, stop, cycles;

    printf("LC-3b-SIM> ");

    scanf("%s", buffer);
    printf("\n");

    switch(buffer[0]) {
    case 'G':
    case 'g':
	go();
	break;

    case 'M':
    case 'm':
	scanf("%i %i", &start, &stop);
	mdump(dumpsim_file, start, stop);
	break;

    case '?':
	help();
	break;
    case 'Q':
    case 'q':
	printf("Bye.\n");
	exit(0);

    case 'R':
    case 'r':
	if (buffer[1] == 'd' || buffer[1] == 'D')
	    rdump(dumpsim_file);
	else {
	    scanf("%d", &cycles);
	    run(cycles);
	}
	break;

    default:
	printf("Invalid Command\n");
	break;
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : init_control_store                              */
/*                                                             */
/* Purpose   : Load microprogram into control store ROM        */ 
/*                                                             */
/***************************************************************/
void init_control_store(char *ucode_filename) {                 
    FILE *ucode;
    int i, j, index;
    char line[200];

    printf("Loading Control Store from file: %s\n", ucode_filename);

    /* Open the micro-code file. */
    if ((ucode = fopen(ucode_filename, "r")) == NULL) {
	printf("Error: Can't open micro-code file %s\n", ucode_filename);
	exit(-1);
    }

    /* Read a line for each row in the control store. */
    for(i = 0; i < CONTROL_STORE_ROWS; i++) {
	if (fscanf(ucode, "%[^\n]\n", line) == EOF) {
	    printf("Error: Too few lines (%d) in micro-code file: %s\n",
		   i, ucode_filename);
	    exit(-1);
	}

	/* Put in bits one at a time. */
	index = 0;

	for (j = 0; j < CONTROL_STORE_BITS; j++) {
	    /* Needs to find enough bits in line. */
	    if (line[index] == '\0') {
		printf("Error: Too few control bits in micro-code file: %s\nLine: %d\n",
		       ucode_filename, i);
		exit(-1);
	    }
	    if (line[index] != '0' && line[index] != '1') {
		printf("Error: Unknown value in micro-code file: %s\nLine: %d, Bit: %d\n",
		       ucode_filename, i, j);
		exit(-1);
	    }

	    /* Set the bit in the Control Store. */
	    CONTROL_STORE[i][j] = (line[index] == '0') ? 0:1;
	    index++;
	}

	/* Warn about extra bits in line. */
	if (line[index] != '\0')
	    printf("Warning: Extra bit(s) in control store file %s. Line: %d\n",
		   ucode_filename, i);
    }
    printf("\n");
}

/************************************************************/
/*                                                          */
/* Procedure : init_memory                                  */
/*                                                          */
/* Purpose   : Zero out the memory array                    */
/*                                                          */
/************************************************************/
void init_memory() {                                           
    int i;

    for (i=0; i < WORDS_IN_MEM; i++) {
	MEMORY[i][0] = 0;
	MEMORY[i][1] = 0;
    }
}

/**************************************************************/
/*                                                            */
/* Procedure : load_program                                   */
/*                                                            */
/* Purpose   : Load program and service routines into mem.    */
/*                                                            */
/**************************************************************/
void load_program(char *program_filename) {                   
    FILE * prog;
    int ii, word, program_base;

    /* Open program file. */
    prog = fopen(program_filename, "r");
    if (prog == NULL) {
	printf("Error: Can't open program file %s\n", program_filename);
	exit(-1);
    }

    /* Read in the program. */
    if (fscanf(prog, "%x\n", &word) != EOF)
	program_base = word >> 1;
    else {
	printf("Error: Program file is empty\n");
	exit(-1);
    }

    ii = 0;
    while (fscanf(prog, "%x\n", &word) != EOF) {
	/* Make sure it fits. */
	if (program_base + ii >= WORDS_IN_MEM) {
	    printf("Error: Program file %s is too long to fit in memory. %x\n",
		   program_filename, ii);
	    exit(-1);
	}

	/* Write the word to memory array. */
	MEMORY[program_base + ii][0] = word & 0x00FF;
	MEMORY[program_base + ii][1] = (word >> 8) & 0x00FF;
	ii++;
    }

    if (CURRENT_LATCHES.PC == 0) CURRENT_LATCHES.PC = (program_base << 1);

    printf("Read %d words from program into memory.\n\n", ii);
}

/***************************************************************/
/*                                                             */
/* Procedure : initialize                                      */
/*                                                             */
/* Purpose   : Load microprogram and machine language program  */ 
/*             and set up initial state of the machine.        */
/*                                                             */
/***************************************************************/
void initialize(char *ucode_filename, char *program_filename, int num_prog_files) { 
    int i;
    init_control_store(ucode_filename);

    init_memory();
    for ( i = 0; i < num_prog_files; i++ ) {
	load_program(program_filename);
	while(*program_filename++ != '\0');
    }
    CURRENT_LATCHES.Z = 1;
    CURRENT_LATCHES.STATE_NUMBER = INITIAL_STATE_NUMBER;
    memcpy(CURRENT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[INITIAL_STATE_NUMBER], sizeof(int)*CONTROL_STORE_BITS);

    NEXT_LATCHES = CURRENT_LATCHES;

    RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[]) {                              
    FILE * dumpsim_file;

    /* Error Checking */
    if (argc < 3) {
	printf("Error: usage: %s <micro_code_file> <program_file_1> <program_file_2> ...\n",
	       argv[0]);
	exit(1);
    }

    printf("LC-3b Simulator\n\n");

    initialize(argv[1], argv[2], argc - 2);

    if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
	printf("Error: Can't open dumpsim file\n");
	exit(-1);
    }

    while (1)
	get_command(dumpsim_file);

}

/***************************************************************/
/* --------- DO NOT MODIFY THE CODE ABOVE THIS LINE -----------*/
/***************************************************************/

/***************************************************************/
/* You are allowed to use the following global variables in your
   code. These are defined above.

   CONTROL_STORE
   MEMORY
   BUS

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */
/***************************************************************/

int ready_mem = 0;
int SReg, DReg;
int SR1Value;

/* Methods taken from old LAB */
int get_bits(int value, int start, int end){
  int result;
  result = value >> end;
  result = result % ( 1 << ( start - end + 1 ) );
  return result;
}

int read_word(int addr){
  return MEMORY[addr>>1][1]<<8 | MEMORY[addr>>1][0];
}


int SEXT(int value, int topbit){
  int shift = sizeof(int)*8 - topbit; 
  return (value << shift )>> shift;
}

int RSHF(int value, int amount, int topbit ){
  int mask;
  mask = 1 << amount;
  mask -= 1;
  mask = mask << ( 16 -amount );

  return ((value >> amount) & ~mask) | ((topbit)?(mask):0); /* TBD */
}



/* Finds value of SR1 and gets the value from REGFile */
void setSR() {
        SR1Value = 0;
        
        if (GetSR1MUX(CURRENT_LATCHES.MICROINSTRUCTION)) {
            SReg = get_bits(CURRENT_LATCHES.IR, 8, 6);
         } else {
            SReg = get_bits(CURRENT_LATCHES.IR, 11, 9);
        }
        SR1Value = CURRENT_LATCHES.REGS[SReg];
        
        
}


/* This function handles the micro_sequencer, allows the multipath data cycle to know which instruction is being handled */

void eval_micro_sequencer() {
    int x0 = 0b000000;
	/* If IRD == 0, Then evaluate JBits based on COND Bits and use it for NEXT STATE */
    if (GetIRD(CURRENT_LATCHES.MICROINSTRUCTION) == 0) {
        int Cond1 = get_bits(GetCOND(CURRENT_LATCHES.MICROINSTRUCTION), 1, 1);
        int Cond0 = get_bits(GetCOND(CURRENT_LATCHES.MICROINSTRUCTION), 0, 0);
        /*Branch /ready /Address mode bits */
        int Branch = (Cond1 & (!Cond0) & CURRENT_LATCHES.BEN) << 2;
        int Ready = ((!Cond1) & Cond0 & CURRENT_LATCHES.READY) << 1;
        int AddrMode = (Cond1 & Cond0 & get_bits(CURRENT_LATCHES.IR,11,11));
	/* OR them all to get one single variable and OR this with Jbits */
        int CondOr = Branch | Ready | AddrMode;       
 
        int JBITS = x0 | GetJ(CURRENT_LATCHES.MICROINSTRUCTION) | CondOr;
        memcpy(NEXT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[JBITS], sizeof(int)*CONTROL_STORE_BITS);
        NEXT_LATCHES.STATE_NUMBER = JBITS; 
        
    } else { 
	/* IF IRD == 1, then NEXT_STATE is IR bits */    
        int IRJ = x0 | get_bits(CURRENT_LATCHES.IR, 15, 12);
        memcpy(NEXT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[IRJ], sizeof(int)*CONTROL_STORE_BITS);
        NEXT_LATCHES.STATE_NUMBER = IRJ; 
    }
    /* Function call to see which source register to use and which DR to use */
    setSR();
}
  /* 
   * Evaluate the address of the next state according to the 
   * micro sequencer logic. Latch the next microinstruction.
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
                    MEMORY[CURRENT_LATCHES.MAR >> 1][1] = (CURRENT_LATCHES.MDR & 0x0000FF00) >> 8;
                    MEMORY[CURRENT_LATCHES.MAR >> 1][0] = (CURRENT_LATCHES.MDR & 0xFF);
                } else if (!dSize && MAR0) {
                    /* BYTE STORE and MAR[0] = 1: MDR[15:8] is written to MEMORY */
                    MEMORY[CURRENT_LATCHES.MAR >> 1][1] = (CURRENT_LATCHES.MDR & 0x0000FF00) >> 8;
                } else if (!dSize && !MAR0) {
                    /* BYTE STORE and MAR[0] = 0; MDR[7:0] is written to MEMORY */
                    MEMORY[CURRENT_LATCHES.MAR >> 1][0] = (CURRENT_LATCHES.MDR & 0xFF);
                }

            }

            /* AFTER READ/WRITE on 5th CYCLE, READY = 0 and READY_MEM back to 0 */ 
            NEXT_LATCHES.READY = 0;
            ready_mem = 0;
        } 
    } 
  /* 
   * This function emulates memory and the WE logic. 
   * Keep track of which cycle of MEMEN we are dealing with.  
   * If fourth, we need to latch Ready bit at the end of 
   * cycle to prepare microsequencer for the fifth cycle.  
   */

}



int busMARMUX, busPC, busALU, busSHF, busMDR, addSum;
void eval_bus_drivers() {
    int *microinst = CURRENT_LATCHES.MICROINSTRUCTION;
    
    /* BUS Value for PC  No Caluculation needed :) */
    busPC = CURRENT_LATCHES.PC;
    
    /* BUS Value for MDR on BUS, Determined on DATA_SIZE(WORD/BYTE) and MAR[0] if BYTE */
    if (GetDATA_SIZE(microinst) == 1) {
            busMDR = Low16bits(CURRENT_LATCHES.MDR); 
    } else {
        if (get_bits(CURRENT_LATCHES.MAR, 0, 0)) {
            busMDR = Low16bits(SEXT(get_bits(CURRENT_LATCHES.MDR,15,8),8));
        } else {
            busMDR = Low16bits(SEXT(CURRENT_LATCHES.MDR & 0xFF, 8));
        }
    }
    
   /* GATE ALU BUS Value Calculation, We check first to see bit 5 of OPCode
    * To determine if we use SR2 or imm5 in the ALU Calculations */ 
    

        int aluk = GetALUK(microinst);
        int SR2Value = 0;
        int bit5 = get_bits(CURRENT_LATCHES.IR, 5,5);
        int bit4 = get_bits(CURRENT_LATCHES.IR, 4,4);        
        
        if (bit5) {
            SR2Value = SEXT(get_bits(CURRENT_LATCHES.IR,4,0),5);
        } else {
            SR2Value = CURRENT_LATCHES.REGS[get_bits(CURRENT_LATCHES.IR,2,0)];
        }
        /* GATE ALU Calculations: 0: ADD, 1: AND 2: XOR 3: PASSA
         * We Low16bits all the values to keep them in 16 bit range
        */
        
        if (aluk == 0) {
            busALU = Low16bits(SR1Value + SR2Value);
        } else if (aluk == 1) {
            busALU = Low16bits(SR1Value & SR2Value);
        } else if (aluk == 2) {
            busALU = Low16bits(SR1Value ^ SR2Value);
        } else {
            busALU = Low16bits(SR1Value);
        }

        /* Here we start calculations for the adder muxes both 1 and 2 so they can be used
        * for the later functions
        */

        /* ADDER1 Calculations, if 0 then PC, if 1 then use value of Base Register */
        int adder1 = GetADDR1MUX(microinst);
        int adder1Val;
        if (adder1 == 0) {
            adder1Val = CURRENT_LATCHES.PC;
        } else {
            adder1Val = SR1Value;
        }

        /* ADDER2MUX Calculations, Setting the value to whichever offset is specified */        
        int adder2 = GetADDR2MUX(microinst);
        int adder2Val;
        if (adder2 == 0) {
            adder2Val = 0;
        } else if (adder2 == 1) {
            adder2Val = SEXT(get_bits(CURRENT_LATCHES.IR, 5, 0),6);
        } else if (adder2 == 2) {
            adder2Val = SEXT(get_bits(CURRENT_LATCHES.IR, 8, 0),9);
        } else {
            adder2Val = SEXT(get_bits(CURRENT_LATCHES.IR, 10, 0),11);
        }
 	
	/* SHIFT if need */
	if (GetLSHF1(microinst)) {
		adder2Val = adder2Val << 1;
	}
        addSum = adder1Val + adder2Val;
       
	
	/* BUS Value for MARMUX */
 
        if (GetMARMUX(microinst)) {
            busMARMUX = addSum;
        } else {
            busMARMUX = (CURRENT_LATCHES.IR & 0xFF) << 1; /* ZEXTED VALUE */ 
        }



        /* BUS Value for GATE_SHF Calulations */

        int amount4 = get_bits(CURRENT_LATCHES.IR, 3,0);
        if (bit4 == 0) {
            busSHF = Low16bits(SR1Value << amount4);
        } else {
            if (bit5) {
                busSHF= RSHF(SR1Value,amount4, get_bits(CURRENT_LATCHES.REGS[SReg], 15, 15));                
            } else {
                busSHF= RSHF(SR1Value,amount4, 0);                
            }   
        }

    
    }
 



void drive_bus() {
    /* We use this microinst variable (pointer) to hold the MICROINSTRUCTION */

    int *microinst = CURRENT_LATCHES.MICROINSTRUCTION;

/*Setting BUS Value to whatever Gate is Open */
/* We look at which GATE is set and assign it to that value. BUS can't be corrupted, set to 0 if none*/
    if (GetGATE_PC(microinst)) {
        BUS = busPC;   
    } else if (GetGATE_ALU(microinst)) {
        BUS = busALU;
    } else if (GetGATE_MARMUX(microinst)) {
        BUS = busMARMUX;
    } else if (GetGATE_SHF(microinst)) {
        BUS = busSHF;
    } else if (GetGATE_MDR(microinst)) {
        BUS = busMDR;
    } else {
        BUS = 0;
    }

  /* 
   * Datapath routine for driving the bus from one of the 5 possible 
   * tristate drivers. This is a short function, we check which of the Gates
   * are set to 1 and use that value to see which value goes on the BUS.
   * If none of the gates, then set the BUS Value to 0 to be safe. 
   */       

}


void latch_datapath_values() {
    int *microinst = CURRENT_LATCHES.MICROINSTRUCTION;

	/* Get which Destination Register to save to*/
	if (GetDRMUX(CURRENT_LATCHES.MICROINSTRUCTION)) {
            DReg = 0x7;
        } else {
            DReg = get_bits(CURRENT_LATCHES.IR,11,9);
        }
	
    /* Latching Datapath values: Latching LD PC */
    if (GetLD_PC(microinst)) {
        switch(GetPCMUX(microinst)) {
            case 0:
                NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.PC +2);
                break;
            case 1:
                NEXT_LATCHES.PC = Low16bits(BUS);
                break;
            case 2:
                NEXT_LATCHES.PC = Low16bits(addSum);
                break;
        }
    } /* Checks if we are adding two to PC, Setting PC to bus or setting PC to Adder MUX SUM  */
   
    /* Latching Datapath values: Latching LD MDR */
	/* Checking if WORD, READ/WRITE AND ODDADDRESS of MAR */
    if (GetLD_MDR(microinst)) {
        if (GetMIO_EN(microinst) == 0) {
            if (GetDATA_SIZE(microinst)) {
                NEXT_LATCHES.MDR = Low16bits(BUS);   
            } else {
                if (CURRENT_LATCHES.MAR & 0x1) {
			int BUS1 = Low16bits(get_bits(BUS,7,0));
                	int oddBUS = Low16bits(BUS1 << 8 | BUS1);
                	NEXT_LATCHES.MDR = oddBUS;
		} else {
                	NEXT_LATCHES.MDR = BUS;
		}
            }   
        }    
    }   

    /* Latching Datapath values: Latching LD BEN */
    int Nbit = get_bits(CURRENT_LATCHES.IR, 11, 11);
    int Zbit = get_bits(CURRENT_LATCHES.IR, 10, 10);
    int Pbit = get_bits(CURRENT_LATCHES.IR, 9, 9);

    if (GetLD_BEN(microinst)){
        NEXT_LATCHES.BEN = ((Nbit & CURRENT_LATCHES.N) | (Zbit & CURRENT_LATCHES.Z) | (Pbit & CURRENT_LATCHES.P));
    }




    /* Latching Datapath values: Latching LD CC */
 	/* Sets all of them to zero and flips accordingly */
    if (GetLD_CC(microinst)) {
        NEXT_LATCHES.N = 0;
        NEXT_LATCHES.Z = 0;
        NEXT_LATCHES.P = 0;
        if (BUS == 0) {
            NEXT_LATCHES.Z = 1;
        } else if (get_bits(BUS,15,15) == 1) {
            NEXT_LATCHES.N  = 1;
        } else if (get_bits(BUS,15,15) == 0) {
            NEXT_LATCHES.P = 1;
        }
    }
        
    /* Latching Datapath values: Latching LD REG */
 
    if (GetLD_REG(microinst)) {
        NEXT_LATCHES.REGS[DReg] = Low16bits(BUS);
    }    

    /* Latching Datapath values: Latching LD IR */
 

    if (GetLD_IR(microinst)) {
        NEXT_LATCHES.IR = Low16bits(BUS);
    }
    
    /* Latching Datapath values: Latching LD MAR */
 
    if (GetLD_MAR(microinst)) {
        NEXT_LATCHES.MAR = Low16bits(BUS);
    }
  /* 
   * Datapath routine for computing all functions that need to latch
   * values in the data path at the end of this cycle.  Some values
   * require sourcing the bus; therefore, this routine has to come 
   * after drive_bus.
   */       

}
