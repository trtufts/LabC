#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>

int iMem[] = (int *) malloc(2048*8*sizeof(int)); //2048 bytes with 8 integers per byte
int dMem[] = (int *) malloc(2048*8*sizeof(int)); //Both memory arrays are the same size

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

//Not done
parser(String instruction){
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

progScanner(string inputFile){
	char *traceEntry;
	FILE *ifp;
	string fullInput = "./" + inputFile;

	traceEntry = malloc(200*sizeof(char));
	
	ifp = fopen(fullInput, "r");

	while (fgets(traceEntry, 75, ifp)) 
		parser(traceEntry);
	fclose(ifp);
}

main(int argc, char* argv[]){
	//Parse input to get file name, Mem time c, Multiply time m, EX op time n
	string inputFile = argv[argc-4];
	int c = atoi(argv[argc-3]);
	int m = atoi(argv[argc-2]);
	int n = atoi(argv[argc-1)];
	
	progScanner(inputFile);
	
}