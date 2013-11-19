#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>

typedef struct{
	int op;												//Op reference: 0 = add, 1 = add1, 2 = sub, 3 = mul, 4 = beq, 5 = lw, 6 = sw, 7 = halt
	int d;												//rd
	int s1;												//rs
	int s2;												//rt
	int im;												//immediate value
} inst;

typedef struct{
	inst pipedInst;										//Latched Instruction in the pipe
	int validItem; 										//0 when the producing stage can act
	int instPC;											//PC of the Latched Instruction
	int operA;											//operand A first operand for the EX stage
	int operB;											//operand B second operand for the EX stage
	int EXresult;										//Result from the EX stage
} latch;

inst *iMem;												//Instruction Memory
int *dMem;												//Data Memory
int *reg;												//Registers
latch IF_ID;											//Latch Definitions
latch ID_EX;
latch EX_MEM;
latch MEM_WB;
inst tempInst;											//Temporary Instruction used to clear instructions
latch tempLatch;										//Temporary Latch used to clear latches
int totalCycles;										//Stage Usage Metric definitions
int IFcycles;
int IDcycles;
int EXcycles;
int MEMcycles;
int WBcycles;
int pc;													//lowercase pc for the main pc, makes it clearly different 
int haltpc;												//stores the pc of the halt instruction, used to stop beq from escaping the instruction set
int branchPending; 										//1 when there is a branch pending, 2 when the branchPending needs to be cleared and 0 otherwise
int IFdelay; 											//Used to simulate long stages
int EXdelay;
int MEMdelay;
int c;
int m;
int n;
int halt;												//1 when the program should be halting, 0 otherwise
int simMode;

void IF(){
	if(IF_ID.pipedInst.op == 7){						//If halt has passed through here, it just auto returns
		return;
	}
	if((IF_ID.validItem == 0)&&(branchPending == 0)){	//If the latch is ready and there is no branch pending, IF acts
		if(iMem[pc].op == 7){							//Halt instruction skips the delay
			IF_ID.pipedInst = iMem[pc];					//Push the instruction into the pipe
			IF_ID.instPC = pc;							//Store the pc with the instruction in the latch
			IF_ID.validItem = 1;						//Mark the latch as ready for consumption
		}
		else if(IFdelay == 1){							//Once the delay count down is finished push to the latch, note that we count this action as the last cycle, so we do the action at delay = 1. This is the same for all other delays in the code
			IF_ID.pipedInst = iMem[pc];					//Push the instruction into the pipe
			IF_ID.instPC = pc;							//Store the pc with the instruction in the latch
			IF_ID.validItem = 1;						//Mark the latch as ready for consumption
			
			pc ++;										//Increment the pc for the next instruction (note we go by one because iMem is indexed by instruction not bits)
			if(pc > haltpc){
				pc = haltpc;							//if the pc goes out of bounds, it sets it back to the halt instruction
			}
			IFcycles ++;								//This is a useful cycle for the IF stage
			IFdelay = c;								//Resets delay for the next instruction
		}
		else{
			IFdelay -= 1;								//Counts down the delay for the IF stage
			IFcycles ++;								//This is a useful cycle for the IF stage
			assert(IFdelay > 0); 						//Catch if this decreases too much
		}
	}else if(branchPending == 2){
		branchPending = 0;
	}
}

int regCheck(int sReg){									//sReg is the register number that is being checked
	if(EX_MEM.validItem == 1){							//This only checks against registers that are in valid latches
		switch(EX_MEM.pipedInst.op){					//Needs to check against different registers for different instructions
			case 0:										//add, sub, mul all produce to the d register
			case 2:
			case 3:
				if(EX_MEM.pipedInst.d == sReg) return 1;
				break;
			case 1:										//addi and lw produce to the s2 register
			case 5:
				if(EX_MEM.pipedInst.s2 == sReg) return 1;
				break;
			case 4:										//beq, sw and halt produce nothing
			case 6:
			case 7:
				break;
			default:
				printf("Op Code not recognized: EX_MEM Latch \n");
				exit(1);								//This should never be reached
		}
	}
	if(MEM_WB.validItem == 1){
		switch(MEM_WB.pipedInst.op){					//Needs to check against different registers for different instructions
			case 0:										//add, sub, mul all produce to the d register
			case 2:
			case 3:
				if(MEM_WB.pipedInst.d == sReg) return 1;
				break;
			case 1:										//addi and lw produce to the s2 register
			case 5:
				if(MEM_WB.pipedInst.s2 == sReg) return 1;
				break;
			case 4:										//beq, sw and halt produce nothing
			case 6:
			case 7:
				break;
			default:
				printf("Op Code not recognized: MEM_WB Latch \n");
				exit(1);								//This should never be reached
		}
	}
	return 0;											//If it never catches, the register is fine
}

void ID(){
	if(ID_EX.pipedInst.op == 7){						//If halt has passed through here, it just auto returns
		return;
	}
	if((IF_ID.validItem == 1)&&(ID_EX.validItem == 0)){	//If both latches are ready go to the instruction switch statements
		if(regCheck(IF_ID.pipedInst.s1) == 1) return;	//Every instruction needs to check the first source register, s1
		switch(IF_ID.pipedInst.op){						//Instruction cases are grouped for efficiency because certain instructions have the same actions
			case 0:										//add
			case 2:										//sub
				if(regCheck(IF_ID.pipedInst.s2) == 1) return;	//Checks s2 for add and sub, these two instructions have the same ID step
				ID_EX = IF_ID;
				ID_EX.operA = reg[IF_ID.pipedInst.s1];	//loads s1 into the next latch
				ID_EX.operB = reg[IF_ID.pipedInst.s2];	//loads s2 into the next latch
				EXdelay = n;							//Set the EXdelay for normal operation delay
				break;
				
			case 3:										//mul			
				if(regCheck(IF_ID.pipedInst.s2) == 1) return;	//Checks s2 for mul, it has a different ID step than add and sub
				ID_EX = IF_ID;
				ID_EX.operA = reg[IF_ID.pipedInst.s1];	//loads s1 into the next latch
				ID_EX.operB = reg[IF_ID.pipedInst.s2];	//loads s2 into the next latch
				EXdelay = m;							//Set the EXdelay for multiplication delay (difference between add and sub for passing to EX)
				break;
				
			case 4:										//beq
				if(regCheck(IF_ID.pipedInst.s2) == 1) return;	//Checks s2 of beq, it has a different ID step than the mul, add and sub
														//check that im value is in range for 16 bit 2's comp, +/- 32767 (2^15 - 1)
				if((IF_ID.pipedInst.im < -32767)||(IF_ID.pipedInst.im > 32767)){
					printf("Immediate field out of numerical bounds, %d \n", IF_ID.pipedInst.im);	//Note we did not just use two assertions because we wanted to print an error message
					exit(1);							//Halt the code if this error message has been printed
				}
				ID_EX = IF_ID;
				ID_EX.operA = reg[IF_ID.pipedInst.s1];	//loads s1 into the next latch
				ID_EX.operB = reg[IF_ID.pipedInst.s2];	//loads s2 into the next latch
				branchPending = 1;						//If it reads a branch, mark branch pending
				EXdelay = n;							//Set the EXdelay for normal operation delay
				break;
				
			case 1:										//addi
			case 5:										//lw
														//check that im value is in range for 16 bit 2's comp, +/- 32767 (2^15 - 1)
				if((IF_ID.pipedInst.im < -32767)||(IF_ID.pipedInst.im > 32767)){
					printf("Immediate field out of numerical bounds, %d \n", IF_ID.pipedInst.im);	//Note we did not just use two assertions because we wanted to print an error message
					exit(1);							//Halt the code if this error message has been printed
				}
				
				ID_EX = IF_ID;
				ID_EX.operA = reg[IF_ID.pipedInst.s1];	//loads s1 into the next latch
				
				EXdelay = n;							//Set the EXdelay for normal operation delay
				break;
				
			case 6:										//sw
				if(regCheck(IF_ID.pipedInst.s2) == 1) return;	//Checks s2 of sw, it relies on s2 while addi and lw do not
														//check that im value is in range for 16 bit 2's comp, +/- 32767 (2^15 - 1)
				if((IF_ID.pipedInst.im < -32767)||(IF_ID.pipedInst.im > 32767)){
					printf("Immediate field out of numerical bounds, %d \n", IF_ID.pipedInst.im);	//Note we did not just use two assertions because we wanted to print an error message
					exit(1);							//Halt the code if this error message has been printed
				}
				ID_EX = IF_ID;
				ID_EX.operA = reg[IF_ID.pipedInst.s1];	//loads s1 into the next latch
				ID_EX.operB = reg[IF_ID.pipedInst.s2];	//loads s2 into the next latch
				EXdelay = n;							//Set the EXdelay for normal operation delay
				break;
				
			case 7:										//halt
				ID_EX = IF_ID;
				EXdelay = 1;							//Set the EXdelay to be 1 for fast propagation
				return;									//Return so we don't count as useful work or clear the latch
				
			default:
				printf("Op Code not recognized: ID Stage \n");
				exit(1);								//If it doesn't decode one of the 7 possibilities there is an error, should be caught by the parser, but this is a backup
				return;
		}
		
		IF_ID = tempLatch;
		IDcycles ++;
	}
}

void EX(){
	if(EX_MEM.pipedInst.op == 7){						//If halt has passed through here, it just auto returns
		return;
	}
	else if(ID_EX.validItem == 1){						//Check previous latch to see if it can start it's operation
		if((EXdelay == 1)&&(EX_MEM.validItem == 0)){ 	//Once the delay count down is finished and the latch is ready to receive, compute and push
			EX_MEM = ID_EX;
			switch(ID_EX.pipedInst.op){
				case 0:									//add
					EX_MEM.EXresult = ID_EX.operA + ID_EX.operB;
					break;
				case 2:									//sub
					EX_MEM.EXresult = ID_EX.operA - ID_EX.operB;
					break;
				case 3:									//mul
					EX_MEM.EXresult = ID_EX.operA * ID_EX.operB;	//Assumes that the system is operating at 32 bits so the result will be cast down to the right size
					break;
				case 4:
					if(ID_EX.operA == ID_EX.operB){
						pc = ID_EX.pipedInst.im + ID_EX.instPC + 1;	//Advances pc by the immediate value past the pc of the next instruction
		 			}
					branchPending = 2;					//Set to 2 so the IF stage knows not to unfreeze until the next cycle
					break;
				case 1:									//addi
				case 5:									//lw
				case 6:									//sw
					EX_MEM.EXresult = ID_EX.operA + ID_EX.pipedInst.im;	//Note these three all do the same thing in EX, the results are just handled differently
					break;
	
				case 7:
					return;
				default:
					printf("Op Code not recognized: EX stage \n");
					exit(1);
			}
			ID_EX = tempLatch;
			EXcycles ++;
		}
		else if(EXdelay != 1){							//Note this is set by the previous stage, so it will never be changed until this stage clears the latch
			EXdelay -= 1;								//Counts down the delay for the EX stage
			EXcycles ++;
			assert(EXdelay > -0); 						//Catch if this decreases too much
		}
	}
}

void MEM(){
	if((EX_MEM.validItem == 1)&&(MEM_WB.validItem == 0)){
		switch(EX_MEM.pipedInst.op){
			case 0:										//add
			case 1:										//addi
			case 2:										//sub
			case 3:										//mul
			case 4:										//beq
				MEM_WB = EX_MEM;
				EX_MEM = tempLatch;
				return;
				
			case 5:										//lw
				if(MEMdelay == 1){						//Once the delay count down is finished push to the latch
					MEM_WB = EX_MEM;
					if(EX_MEM.EXresult % 4 != 0){		//Makes sure the memory alignment is correct
						printf("Data Alignment Error, %d not divisible by 4 \n", EX_MEM.EXresult);
						exit(1);						
					}
					MEM_WB.EXresult = dMem[EX_MEM.EXresult/4];	//Our dMem is indexed by words, so we divide bytes by 4 to get word indexing. Loads into the EXresult to be loaded into a register in WB
					
					MEMdelay = c;						//Resets delay for the next instruction
					EX_MEM = tempLatch;
					MEMcycles ++;
				}
				else{
					MEMdelay -= 1;						//Counts down the delay for the MEM stage
					MEMcycles ++;
					assert(MEMdelay > 0); 				//Catch if this decreases too much
				}
				return;
				
			case 6:										//sw
				if(MEMdelay == 1){						//Once the delay count down is finished push to the latch
					MEM_WB = EX_MEM;
					if(EX_MEM.EXresult % 4 != 0){		//Makes sure the memory alignment is correct
						printf("Data Alignment Error, %d not divisible by 4 \n", EX_MEM.EXresult);
						exit(1);						
					}
					dMem[EX_MEM.EXresult/4] = reg[EX_MEM.pipedInst.s2];	//Stores the proper register into dMem
					
					MEMdelay = c;						//Resets delay for the next instruction
					EX_MEM = tempLatch;
					MEMcycles ++;
				}
				else{
					MEMdelay -= 1;						//Counts down the delay for the MEM stage
					MEMcycles ++;
					assert(MEMdelay > 0); 				//Catch if this decreases too much
				}
								
				return;										
			
			case 7:										//halt
				MEM_WB.pipedInst = EX_MEM.pipedInst;	//Push the halt instruction down the pipe
				MEM_WB.instPC = EX_MEM.instPC;			//Push the pc for good measure
				MEM_WB.validItem = 1;					//Mark the latch as ready for consumption so the halt propagates
				return;
			default:
				printf("Op code not recognized: WB stage \n");
				exit(1);								//Catches errors in op codes
		}
	}
}

void WB(){
	if(MEM_WB.validItem == 1){
		switch(MEM_WB.pipedInst.op){
			case 0:										//add
			case 2:										//sub
			case 3:										//mul
				reg[MEM_WB.pipedInst.d] = MEM_WB.EXresult;	//Stores the result in the proper destination register
				break;
			
			case 1:										//addi
			case 5:										//lw
				reg[MEM_WB.pipedInst.s2] = MEM_WB.EXresult;	//Stores the result in the proper destination register
				break;
			
			case 4:										//beq
			case 6:										//sw
			case 7:										//halt
				MEM_WB = tempLatch;
				return;									//These instructions don't count as cycles for WB
			default:
				printf("Op code not recognized: WB stage \n");
				exit(1);								//Catches errors in op codes
		}
		MEM_WB = tempLatch;
		WBcycles ++;
	}
}

int getRegNumber(char * inputReg){
	if((strcmp(inputReg,"zero")==0)||(strcmp(inputReg,"0")==0)){
		return 0;
	}else if((strcmp(inputReg,"at")==0)||(strcmp(inputReg,"1")==0)){
		return 1;
	}else if((strcmp(inputReg,"v0")==0)||(strcmp(inputReg,"2")==0)){
		return 2;
	}else if((strcmp(inputReg,"v1")==0)||(strcmp(inputReg,"3")==0)){
		return 3;
	}else if((strcmp(inputReg,"a0")==0)||(strcmp(inputReg,"4")==0)){
		return 4;
	}else if((strcmp(inputReg,"a1")==0)||(strcmp(inputReg,"5")==0)){
		return 5;
	}else if((strcmp(inputReg,"a2")==0)||(strcmp(inputReg,"6")==0)){
		return 6;
	}else if((strcmp(inputReg,"a3")==0)||(strcmp(inputReg,"7")==0)){
		return 7;
	}else if((strcmp(inputReg,"t0")==0)||(strcmp(inputReg,"8")==0)){
		return 8;
	}else if((strcmp(inputReg,"t1")==0)||(strcmp(inputReg,"9")==0)){
		return 9;
	}else if((strcmp(inputReg,"t2")==0)||(strcmp(inputReg,"10")==0)){
		return 10;
	}else if((strcmp(inputReg,"t3")==0)||(strcmp(inputReg,"11")==0)){
		return 11;
	}else if((strcmp(inputReg,"t4")==0)||(strcmp(inputReg,"12")==0)){
		return 12;
	}else if((strcmp(inputReg,"t5")==0)||(strcmp(inputReg,"13")==0)){
		return 13;
	}else if((strcmp(inputReg,"t6")==0)||(strcmp(inputReg,"14")==0)){
		return 14;
	}else if((strcmp(inputReg,"t7")==0)||(strcmp(inputReg,"15")==0)){
		return 15;
	}else if((strcmp(inputReg,"s0")==0)||(strcmp(inputReg,"16")==0)){
		return 16;
	}else if((strcmp(inputReg,"s1")==0)||(strcmp(inputReg,"17")==0)){
		return 17;
	}else if((strcmp(inputReg,"s2")==0)||(strcmp(inputReg,"18")==0)){
		return 18;
	}else if((strcmp(inputReg,"s3")==0)||(strcmp(inputReg,"19")==0)){
		return 19;
	}else if((strcmp(inputReg,"s4")==0)||(strcmp(inputReg,"20")==0)){
		return 20;
	}else if((strcmp(inputReg,"s5")==0)||(strcmp(inputReg,"21")==0)){
		return 21;
	}else if((strcmp(inputReg,"s6")==0)||(strcmp(inputReg,"22")==0)){
		return 22;
	}else if((strcmp(inputReg,"s7")==0)||(strcmp(inputReg,"23")==0)){
		return 23;
	}else if((strcmp(inputReg,"t8")==0)||(strcmp(inputReg,"24")==0)){
		return 24;
	}else if((strcmp(inputReg,"t9")==0)||(strcmp(inputReg,"25")==0)){
		return 25;
	}else if((strcmp(inputReg,"k0")==0)||(strcmp(inputReg,"26")==0)){
		return 26;
	}else if((strcmp(inputReg,"k1")==0)||(strcmp(inputReg,"27")==0)){
		return 27;
	}else if((strcmp(inputReg,"gp")==0)||(strcmp(inputReg,"28")==0)){
		return 28;
	}else if((strcmp(inputReg,"sp")==0)||(strcmp(inputReg,"29")==0)){
		return 29;
	}else if((strcmp(inputReg,"fp")==0)||(strcmp(inputReg,"30")==0)){
		return 30;
	}else if((strcmp(inputReg,"ra")==0)||(strcmp(inputReg,"31")==0)){
		return 31;
	}else if(atoi(inputReg)>31){
		printf("Register out of bounds, %d is higher than 31 \n", atoi(inputReg));
		exit(1);
	}else {
		printf("Register not recognized, %s is not a valid register \n", inputReg);
		exit(1);
	}
}

inst parser(char * instruction){
	int i;
	char delimiters[]="$, \n()";  						//Define delimiters for the strtok functions
	char ** instructionFields; 							//Define the resulting instruction fields
	inst parsedInst = {0, 0, 0, 0, 0};
	int format;											//0 for R type, 1 for I type, 2 for lw/sw, 3 for halt

	//parse first field to find opcode
	for(i=0; i < 4; i++){
		*(instructionFields+i) = (char *) malloc(20*sizeof(char *));
	}
	instructionFields[0] = strtok(instruction, delimiters);
	instructionFields[1] = strtok(NULL, delimiters);
	instructionFields[2] = strtok(NULL, delimiters);
	instructionFields[3] = strtok(NULL, delimiters);	
	
	if(strcmp(instructionFields[0],"add")==0){			//Compares the first field against the valid opcodes to get type and store the appropriate number
		format = 0;
		parsedInst.op = 0;
	}else if(strcmp(instructionFields[0],"addi")==0){
		format = 1;
		parsedInst.op = 1;
	}else if(strcmp(instructionFields[0],"sub")==0){
		format = 0;
		parsedInst.op = 2;
	}else if(strcmp(instructionFields[0],"mul")==0){
		format = 0;
		parsedInst.op = 3;
	}else if(strcmp(instructionFields[0],"beq")==0){
		format = 1;
		parsedInst.op = 4;
	}else if(strcmp(instructionFields[0],"lw")==0){
		format = 2;
		parsedInst.op = 5;
	}else if(strcmp(instructionFields[0],"sw")==0){
		format = 2;
		parsedInst.op = 6;
	}else if(strcmp(instructionFields[0],"haltSimulation")==0){
		format = 3;
		parsedInst.op = 7;
	}else{
		printf("Op code %s not recognized: Parser \n", instructionFields[0]);
		exit(1);
	}
	
	switch(format){
		case 0:
			parsedInst.d = getRegNumber(instructionFields[1]);
			parsedInst.s1 = getRegNumber(instructionFields[2]);
			parsedInst.s2 = getRegNumber(instructionFields[3]);
			return parsedInst;
		case 1:
			parsedInst.s2 = getRegNumber(instructionFields[1]);
			parsedInst.s1 = getRegNumber(instructionFields[2]);
			parsedInst.im = atoi(instructionFields[3]);
			return parsedInst;
		case 2:
			parsedInst.s2 = getRegNumber(instructionFields[1]);
			parsedInst.s1 = getRegNumber(instructionFields[3]);
			parsedInst.im = atoi(instructionFields[2]);
			return parsedInst;
		case 3:
			return parsedInst;
		default:
			printf("Format not recognized \n");
			exit(1);
	}
	
	free(instructionFields);
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


int main(int argc, char* argv[]){
	//instantiate the Mem's
	iMem = (inst *) malloc(512*sizeof(inst));			//iMem can hold 512 instructions so we index it as such
	dMem = (int *) malloc(512*sizeof(int)); 			//Data memory is stored as words since that is the smallest unit we will be using here
	int i;
	for(i=0; i < 512; i++){
		dMem[i] = 0;
	}
	
	//instantiate the registers
	reg = (int *) malloc(32*sizeof(int));
	for(i=0; i < 32; i++){
		reg[i] = 0;
	}
	
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
	tempInst = (inst) {0, 0, 0, 0, 0};
	tempLatch = (latch) {tempInst, 0, 0, 0, 0, 0};
	IF_ID = tempLatch;
	ID_EX = tempLatch;
	EX_MEM = tempLatch;
	MEM_WB = tempLatch;
	
	//Parse inputs to get file name, Mem time c, Multiply time m, EX op time n
	char * inputFileName = (char *) malloc(20*sizeof(char));
	strcpy(inputFileName, argv[argc-5]);
	c = atoi(argv[argc-4]);
	m = atoi(argv[argc-3]);
	n = atoi(argv[argc-2]);
	simMode = atoi(argv[argc-1]);						//0 for single cycle, 1 for batch
	IFdelay = c;
	EXdelay = 1;
	MEMdelay = c;
	
	
	assert(c>0);										//These three variables should be greater than 0, or else it causes problems later
	assert(m>0);
	assert(n>0);

	assert((simMode==0)||(simMode==1));					//There are only two options for simMode, catches if there is an error in this assignment
	
	
	//Instruction Parsing block
	char * instruction = (char *) malloc(100*sizeof(char));
	char * fullInput = (char *) malloc(30*sizeof(char));
	strcpy(fullInput, "./");
	strcat(fullInput, inputFileName);
	FILE * inputFile = fopen(fullInput, "r");
	do{
		instruction = progScanner(inputFile, instruction);
		tempInst = parser(instruction);
		iMem[pc] = tempInst;
		pc += 1;
		haltpc = pc-1;									//assigns for every pc, the last assignment will be the halt code
		assert(pc < 512); 								//Only 512 instructions fit in the memory
	} while(tempInst.op != 7);
	fclose(inputFile);
	
	//Reset tempInst so it can be used to clear latches
	tempInst = (inst) {0, 0, 0, 0, 0};
	
	pc = 0; 											//reset pc to 0
	
	while((IF_ID.pipedInst.op != 7)||(ID_EX.pipedInst.op != 7)||(EX_MEM.pipedInst.op != 7)||(MEM_WB.pipedInst.op != 7)){
		WB();
		MEM();
		EX();
		ID();
		IF();
		totalCycles++;
		if(simMode == 1){
			for(i=1;i<32;i++){
				printf("Register %d contains %d \n", i, reg[i]);
			}
			printf("PC is %d \n", pc);
			getchar();
		}
	}
	printf("Total Cycles: %d \n", totalCycles);
	printf("IF Utilization: %d\% \n", ((IFcycles * 100)/totalCycles));
	printf("ID Utilization: %d\% \n", ((IDcycles * 100)/totalCycles));
	printf("EX Utilization: %d\% \n", ((EXcycles * 100)/totalCycles));
	printf("MEM Utilization: %d\% \n", ((MEMcycles * 100)/totalCycles));
	printf("WB Utilization: %d\% \n", ((WBcycles * 100)/totalCycles));
	
	for(i=1;i<32;i++){
		printf("Register %d contains %d \n", i, reg[i]);
	}
	printf("PC is %d \n", pc);
	
}