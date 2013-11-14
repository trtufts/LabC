#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>

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
    char delimiters[]=",";  // Define delimiters for the strtok functions
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

//Not done
progScanner(String inputFile){
    char *traceEntry1;
    FILE *ifp;

    traceEntry1 = malloc(200*sizeof(char));
    ifp = fopen("./program.txt", "r");

    while (fgets(traceEntry1, 75, ifp)) 
      printf("String input is %s \n", traceEntry1);
    fclose(ifp);
}

main(){
	//Parse input to get file name, Mem time c, Multiply time m, EX op time n
	progScanner(inputFile);
	

}