#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>

int iMem[] = (int *) malloc(2048*sizeof(int)); //2048 Bytes 
int dMem[] = (int *) malloc(2048*sizeof(int)); //Both memory arrays are the same size

struct inst{
	int op, s1, s2, d, im;
}

IF(int c){
//c cycles
}

ID(){
//One cycle
}

EX(int n, int m){
//n cycles or m cycles for multiply
}

MEM(int c){
//c cycles
}

WB(){
//One cycle
}

//Needs to pick R or I type, and parse stuff into numbers for iMem
struct inst parser(String instruction){
	int i;
	char delimiters[]=", ";  // Define delimiters for the strtok functions
	char ** instructionFields; //Define the resulting instruction fields
	char format;

	//parse first field to find opcode
	*instructionFields = (char *) malloc(6*sizeof(char*));
	instructionFields[0] = strtok(instruction, delimiters);
	//add, sub, mul are R type
	//addi, lw, sw, beq are I type
	//make format R or I
	
	if(format == 'R'){
		for (i=1; i<4; i++)
			*(instructionFields+i) = (char *) malloc(20*sizeof(char *));
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

//almost done, needs fgets tuning
char * progScanner(string inputFile){
	char *traceEntry;
	FILE *ifp;
	string fullInput = "./" + inputFile;

	traceEntry = malloc(200*sizeof(char));
	
	ifp = fopen(fullInput, "r");

	while (fgets(traceEntry, 100, ifp)) 
		parser(traceEntry);
	fclose(ifp);
}

main(int argc, char* argv[]){
	//instantiate the dMem to all 0's, 2048 bytes to set
	
	//instantiate the registers
	int reg[] = (int *) malloc(32*sizeof(int));
	reg[0] = 0;

	//instantiates the utilization data collection variables
	int totalCycles = 0;
	int IFcycles = 0;
	int IDcycles = 0;
	int EXcycles = 0;
	int MEMcycles = 0;
	int WBcycles = 0;
	
	
	
	//Parse input to get file name, Mem time c, Multiply time m, EX op time n
	string inputFile = argv[argc-4];
	int c = atoi(argv[argc-3]);
	int m = atoi(argv[argc-2]);
	int n = atoi(argv[argc-1)];
	
	progScanner(inputFile);
	
}