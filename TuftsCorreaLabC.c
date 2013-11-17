#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>

enum inst {ADD,ADDI,SUB,MULT,BEQ,LW,SW,HALT};

//Consider changinng to typedef to remove struct code clutter
struct inst{
	int op;
	int d;
	int s1;
	int s2;
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
struct inst tempInst;
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
	if(IF_ID.pipedInst == 7){							//If halt has passed through here, it just auto returns
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
			assert(IFdelay > -1); 						//Catch if this decreases too much
		}
		
	}
}

int regCheck(int sReg, int regType){					//sReg is the register number, regType = 1 is s1, regType = 2 is s2
	assert((regType != 1)&&(regType != 2));				//if the regType is not 1 or 2 this function is being used wrong
	if(regType = 1){
		if((EX_MEM.validItem == 1)&&(EX_MEM.s1 == sReg)){	//This only checks against registers that are in valid latches
			return 1;
		}
		if((MEM_WB.validItem == 1)&&(MEM_WB.s1 == sReg)){
			return 1;
		}
	}
	if(regType = 2){
		if((EX_MEM.validItem == 1)&&(EX_MEM.s2 == sReg)){	//This only checks against registers that are in valid latches
			return 1;
		}
		if((MEM_WB.validItem == 1)&&(MEM_WB.s2 == sReg)){
			return 1;
		}
	}
	
	return 0;											//If it never catches, the register is fine
}

void ID(){
	if(ID_EX.pipedInst == 7){							//If halt has passed through here, it just auto returns
		return;
	}
	if((IF_ID.validItem == 1)&&(ID_EX.validItem == 0)){	//If both latches are ready go to the instruction switch statements
		if(regCheck(IF_ID.pipedInst.s1, 1) == 1) return;		//Every instruction needs to check the first source register, s1
		switch(IF_ID.pipedInst.op){						//Instruction cases are grouped for efficiency because certain instructions have the same actions
			case 0:										//add
			case 2:										//sub
				if(regCheck(IF_ID.pipedInst.s2, 2) == 1) return;	//Checks s2 for add and sub, if they pass they have the same ID step
				
				break;
			case 3:										//mul			
				if(regCheck(IF_ID.pipedInst.s2, 2) == 1) return;	//Checks s2 for mul, it has a different ID step than add and sub
				break;
			case 4:										//beq
				if(regCheck(IF_ID.pipedInst.s1, 1) == 1) return;	//Checks s2 of beq, it has a different ID step than the mul, add and sub
				branchPending = 1;						//If it reads a branch, mark branch pending
				break;
			case 1:										//addi
			case 5:										//lw
			
				break;
			case 6:										//sw
				if(regCheck(IF_ID.pipedInst.s1, 1) == 1) return;	//Checks s2 of sw, it relies on s2 while addi and lw do not
				
				break;
			case 7:										//halt
				ID_EX.pipedInst = IF_ID.pipedInst;		//Push the halt instruction down the pipe
				ID_EX.instPC = IF_ID.instPC;			//Push the pc for good measure
				ID_EX.validItem = 1;					//Mark the latch as ready for consumption so the halt propagates
				nextEXdelay = 1;						//Set the EXdelay to be 1 for fast propagation
				return;
				
			default:
				printf("Op Code not recognized: ID Stage");
				assert(0 == 1);							//If it doesn't decode one of the 7 possibilities there is an error, should be caught by the parser, but this is a backup
				return;
		}
		
		IF_ID.validItem = 0;
		IF_ID.pipedInst = tempInst;
		IF_ID.instPC = 0;
		IDcycles ++;
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
	tempInst = {0, 0, 0, 0, 0};
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
	
	//Reset tempInst so it can be used to clear latches
	tempInst = {0, 0, 0, 0, 0};
	
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