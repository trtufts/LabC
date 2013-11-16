#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>

struct inst iMem[];
int dMem[];
int reg[];
int totalCycles;
int IFcycles;
int IDcycles;
int EXcycles;
int MEMcycles;
int WBcycles;
int pc;

enum inst {ADD,ADDI,SUB,MULT,BEQ,LW,SW,HALT};

//Consider changinng to typedef to remove struct code clutter
struct inst{
	int op, s1, s2, d, im;
};

void IF(int c){
//c cycles
}

void ID(){
//One cycle
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
	dMem = malloc(2048*sizeof(int)); //Both memory arrays are the same size
	
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
	
	//Parse input to get file name, Mem time c, Multiply time m, EX op time n
	string inputFileName = argv[argc-4];
	int c = atoi(argv[argc-3]);
	int m = atoi(argv[argc-2]);
	int n = atoi(argv[argc-1)];
	
	//Instruction Parsing block
	char * instruction = malloc(100*sizeof(char);
	string fullInput = strcat("./", inputFileName);
	FILE * inputFile = fopen(fullInput, "r");
	struct inst tempInst;
	do{
		instruction = progScanner(inputFile, instruction);
		tempInst = parser(instruction);
		
	} while(tempInst.op != 7);
	fclose(inputFile);
	
	
}