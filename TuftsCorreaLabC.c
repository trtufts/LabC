#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>

enum inst {ADD,ADDI,SUB,MULT,BEQ,LW,SW,HALT};

//Consider changinng to typedef to remove struct code clutter
struct inst{
	int op;
	int s1;
	int s2;
	int d;
	int im;
};

struct latch{
	struct inst pipedInst;
	int validItem; //0 when the producing stage can act
	int instPC;
}

struct inst iMem[];
int dMem[];
int reg[];
struct latch IF_ID;
struct latch ID_EX;
struct latch EX_MEM;
struct latch MEM_WB;
int totalCycles;
int IFcycles;
int IDcycles;
int EXcycles;
int MEMcycles;
int WBcycles;
int pc;	//lowercase pc for the main pc, instPC for values sent down the pipe, for clarity
int branchPending; //1 when there is a branch pending, 0 otherwise
int IFdelay; //Used to simulate long stages
int EXdelay;
int nextEXdelay; //Used to dynamically assign delay to the EX stage
int MEMdelay;
int c;
int m;
int n;
int halt;	//1 when the program should be halting, 0 otherwise
int simMode;

void IF(){
	if(IF_ID.pipedInst == 7){	//If halt has passed through here, it just auto returns
		return;
	}
	if((IF_ID.validItem == 0)&&(branchPending == 0)){	//If the latch is ready and there is no branch pending, IF acts
		if(IFdelay == 0){								//Once the delay count down is finished push to the latch
			IF_ID.pipedInst = iMem(pc);					//Push the instruction into the pipe
			IF_ID.instPC = pc;							//Store the pc with the instruction in the latch
			IF_ID.validItem = 1;						//Mark the latch as ready for consumption
			
			pc ++;										//Increment the pc for the next instruction (note we go by one because iMem is indexed by instruction not bits)
			IFcycles ++;								//This is a useful cycle for the IF stage
			IFdelay == c;								//Resets delay for the next instruction
		}
		else{
			IFdelay -= 1;								//Counts down the delay for the IF stage
			IFcycles ++;								//This is a useful cycle for the IF stage
			assert(IFdelay > -1); //Catch if this decreases too much
		}
		
	}
}

void ID(){
//One cycle

		if(IF_ID.pipedInst.op == 4){				//If it reads a branch, mark branch pending
			branchPending = 1;
		}
}

void EX(int n, int m){
//n cycles or m cycles for multiply
}

void MEM(int c){
//c cycles
}

void WB(){
//One cycle
}

//Needs to pick R or I type, and parse stuff into numbers for iMem
struct inst parser(char * instruction){
	int i;
	char delimiters[]=", ";  // Define delimiters for the strtok functions
	char ** instructionFields; //Define the resulting instruction fields
	char format;

	//parse first field to find opcode
	*instructionFields = malloc(6*sizeof(char*));
	instructionFields[0] = strtok(instruction, delimiters);
	//add, sub, mul are R type
	//addi, lw, sw, beq are I type
	//make format R or I
	
	if(format == 'R'){
		for (i=1; i<4; i++)
			*(instructionFields+i) = malloc(20*sizeof(char *));
	}
	else if(format == 'I'){
	}
	else{
	
	}

	printf("inputString[]=%s\n", inputString);
	instructionFields[0] = strtok(inputString, delimiters);
	printf("inputString[]=%s\n", inputString);
	instructionFields[1] = strtok(NULL, delimiters);
	printf("inputString[]=%s\n", inputString);
	instructionFields[2] = strtok(NULL, delimiters);
	printf("inputString[]=%s\n", inputString);
	printf("%s \n%s \n%s \n", instructionFields[0], instructionFields[1], instructionFields[2]);
}

//should be functional
char * progScanner(FILE *inputFile, char * instruction){
	if(fgets(instruction, 100, inputFile) != NULL){
		return instruction;
	}
	else{
		return NULL;
	}
}


main(int argc, char* argv[]){
	//instantiate the Mem's
	iMem = malloc(512*sizeof(struct int)); //1 instruction takes up 1 word, so iMem is an array of 512 instructions
	dMem = malloc(512*sizeof(int)); //Data memory is stored as words since that is the smallest unit we will be using here
	int i;
	for(i=0; i < 512; i++){
		dMem[i] = 0;
	}
	
	
	//instantiate the registers
	reg = malloc(32*sizeof(int));
	reg[0] = 0;

	//instantiates the utilization data collection variables
	totalCycles = 0;
	IFcycles = 0;
	IDcycles = 0;
	EXcycles = 0;
	MEMcycles = 0;
	WBcycles = 0;
	
	//instantiates general function variables
	pc = 0;
	halt = 1;
	struct inst tempInst = {0, 0, 0, 0, 0};
	IF_ID = {tempInst, 0, 0};
	ID_EX = {tempInst, 0, 0};
	EX_MEM = {tempInst, 0, 0};
	MEM_WB = {tempInst, 0, 0};
	
	//Parse inputs to get file name, Mem time c, Multiply time m, EX op time n
	string inputFileName = argv[argc-4];
	c = atoi(argv[argc-3]);
	m = atoi(argv[argc-2]);
	n = atoi(argv[argc-1)];
	IFdelay = c;
	EXdelay = 0;
	nextEXdelay = 0;
	MEMdelay = c;
	
	
	assert(c>0);	//These three variables should be greater than 0, or else it causes problems later
	assert(m>0);
	assert(n>0);

	assert((simMode==0)||(simMode==1));	//There are only two options for simMode, catches if there is an error in this assignment
	
	
	//Instruction Parsing block
	char * instruction = malloc(100*sizeof(char);
	string fullInput = strcat("./", inputFileName);
	FILE * inputFile = fopen(fullInput, "r");
	do{
		instruction = progScanner(inputFile, instruction);
		tempInst = parser(instruction);
		iMem[pc] = tempInst;
		pc += 1;
		assert(pc < 512); //Only 512 instructions fit in the memory
	} while(tempInst.op != 7);
	fclose(inputFile);
	
	pc = 0; //reset pc to 0
	
	while((halt != 1)&&(IF_ID.pipedInst.op != 7)&&(ID_EX.pipedInst.op != 7)&&(EX_MEM.pipedInst.op != 7)&&(MEM_WB.pipedInst.op != 7)){
		WB();
		MEM();
		EX();
		ID();
		IF();
		totalCycles++;
		
	}
	
	
}