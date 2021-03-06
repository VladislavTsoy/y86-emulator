#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "y86emul.h"

/*
[0] = %eax; [4] = %esp;
[1] = %ecx; [5] = %epb;
[2] = %edx; [6] = %esi;
[3] = %ebx; [7] = %edi;
*/
int registers[8]; 
char *memoryArray;
/*
conditionFlags[0] = ZF;
conditionFlags[1] = SF;
conditionFlags[2] = OF;
*/
char conditionFlags[3];
int PC;
int c = 0;
int rA;
int rB;
int memorySize;
/*status codes*/
enum statusCode {
	AOK, 
	HLT, 
	ADR, 
	INS 
} status ;

typedef struct {
	unsigned char lowerBit: 4;
	unsigned char upperBit: 4;
} charByte;

/*checks byte, makes sure its an appropriate register*/
void check_rA() {
	charByte *registerByte = (charByte*)(memoryArray + PC + 1);
	int rA = registerByte->upperBit;

	if (rA >= 0 && rA <=7) {
		status = AOK;
	} else {
		status = INS;
	}
}

/*checks byte, makes sure its an appropriate register*/
void check_rB() {
	charByte *registerByte = (charByte*)(memoryArray + PC + 1);
	int rB = registerByte->lowerBit;

	if (rB >= 0 && rB <=7) {
		status = AOK;
	} else {
		status = INS;
	}
}

/*checks byte, makes sure it is 0x0f*/
void check_F() {
	charByte *registerByte = (charByte*)(memoryArray + PC + 1);
	int lf = registerByte->lowerBit;
	int uf = registerByte->upperBit;

	if (lf == 0xf || uf == 0xf){
		status = AOK;
	} else {
		status = INS;
	}
}
/*seg fault check, makes sure the  address given is within the scope*/
void check_address(int *address){
	if (memorySize <= *address || 0 > *address) {
		status = ADR;
	} else {
		status = AOK;
	}
}

/*gets the register rA byte*/
int getRegisterA(unsigned char * byte) {
	charByte *registerByte = (charByte*)(memoryArray + PC + 1);
	int rA = registerByte->upperBit;
	return rA;
}
/*gets the register rB byte*/
int getRegisterB(unsigned char* byte) {
	charByte *registerByte = (charByte*)(memoryArray + PC + 1);
	int rB = registerByte->lowerBit;
	return rB;
}

/*executes register to register move*/
void execute_rrmovl(unsigned char * byte){
	check_rA();
	check_rB();
	if(status == AOK) {
		rA = getRegisterA(byte);
		rB = getRegisterB(byte);
		registers[rB] = registers[rA];
		PC += 2;
		
	}
}
/*executes immediate to register move*/
void execute_irmovl(unsigned char * byte){
	check_F();
	check_rB();
	if (status == AOK) {
		rB = getRegisterB(byte);
		int *value = (int *)(memoryArray+PC+2);
		registers[rB] = *value;	
	}
	PC += 6;
	
}
/*executes register to memory move*/
void execute_rmmovl(unsigned char * byte){
	check_rA();
	check_rB();

	if(status == AOK) {
		rA = getRegisterA(byte);
		rB = getRegisterB(byte);
		int *displacement = (int *)(memoryArray+PC+2);
		check_address((registers[rB] + displacement));
		memoryArray[registers[rB] + *displacement] = registers[rA];
	}
	PC += 6;
}
/*executes memory to register move*/
void execute_mrmovl(unsigned char * byte){
	check_rA();
	check_rB();
	if(status == AOK) {
		rA = getRegisterA(byte);
		rB = getRegisterB(byte);
		int *displacement = (int *)(memoryArray+PC+2);
		registers[rA] = memoryArray[registers[rB] + *displacement];
	}
	PC += 6;
}
/*executes operations according to op code and sets conditional flags*/
void execute_OP1(unsigned char * byte){
	check_rA();
	check_rB();
	if (status == AOK) {
		switch(*byte) {
			case 0x60:
				rA = getRegisterA(byte);
				rB = getRegisterB(byte);
				int tempRb = registers[rB];
				registers[rB] = registers[rA] + registers[rB];

				if(registers[rB] == 0) {
					conditionFlags[0] = 1;
				} else {
					conditionFlags[0] = 0;
				}
				if(registers[rB] < 0) {
					conditionFlags[1] = 1;
				} else {
					conditionFlags[1] = 0;
				}
				if((registers[rA] > 0 && tempRb > 0 && registers[rB] < 0) 
				|| (registers[rA] < 0 && tempRb < 0 && registers[rB] > 0)){
					conditionFlags[2] = 1;
				} else {
					conditionFlags[2] = 0;
				}
				PC += 2;
				break;	

			case 0x61:
				rA = getRegisterA(byte);
				rB = getRegisterB(byte);
				tempRb = registers[rB];
				registers[rB] = registers[rB] - registers[rA];
				if(registers[rB] == 0) {
					conditionFlags[0] = 1;
				} else {
					conditionFlags[0] = 0;
				}
				if (registers[rB] < 0) {
					conditionFlags[1] = 1;
				} else { 
					conditionFlags[1] = 0;
				}
				if((registers[rA] < 0 && tempRb > 0 && registers[rB] < 0) 
				|| (registers[rA] > 0 && tempRb < 0 && registers[rB] > 0)) {
					conditionFlags[2] = 1;
				} else {
					conditionFlags[2] = 0;
				}
				PC += 2;

				break;
			case 0x62:
				rA = getRegisterA(byte);
				rB = getRegisterB(byte);
				registers[rB] = registers[rB] & registers[rA];
				if(registers[rB] == 0) {
					conditionFlags[0] = 1;
				} else {
					conditionFlags[0] = 0;
				}
				if(registers[rB] < 0) {
					conditionFlags[1] = 1;
				} else {
					conditionFlags[1] = 0;
				}
				conditionFlags[2] = 0;
				PC += 2;
				break;
			case 0x63:
				rA = getRegisterA(byte);
				rB = getRegisterB(byte);
				registers[rB] = registers[rB] ^ registers[rA];
				if(registers[rB] == 0) {
					conditionFlags[0] = 1;
				} else {
					conditionFlags[0] = 0;
				}
				if(registers[rB] < 0) {
					conditionFlags[1] = 1;
				} else {
					conditionFlags[1] = 0;
				}
				conditionFlags[2] = 0;
				PC += 2;
				break;
			case 0x64:
				rA = getRegisterA(byte);
				rB = getRegisterB(byte);
				tempRb = registers[rB];
				registers[rB] = registers[rB] * registers[rA];
				if(registers[rB] == 0) {
					conditionFlags[0] = 1;
				} else {
					conditionFlags[0] = 0;
				}
				if (registers[rB] < 0) {
					conditionFlags[1] = 1;
				} else {
					conditionFlags[1] = 0;
				} 
				if((registers[rA] > 0 && tempRb > 0 && registers[rB] < 0) 
				|| (registers[rA] < 0 && tempRb < 0 && registers[rB] < 0)

				|| (registers[rA] < 0 && tempRb > 0 && registers[rB] > 0) 
				|| (registers[rA] > 0 && tempRb < 0 && registers[rB] > 0)) {
					conditionFlags[2] = 1;
				} else {
					conditionFlags[2] = 0;
				}
				PC += 2;
				break;

			case 0x65:	
				rA = getRegisterA(byte);
				rB = getRegisterB(byte);
				int cmp = registers[rB] - registers[rA];
				if(cmp == 0){
					conditionFlags[0] = 1;
				} else {
					conditionFlags[0] = 0;
				}
				if((registers[rA] < 0 && registers[rB] > 0 && cmp < 0) 
				|| (registers[rA] > 0 && registers[rB] < 0 && cmp > 0)) {
					
					conditionFlags[2] = 1;
				} else {
					conditionFlags[2] = 0;
				}
				if (cmp < 0){
					conditionFlags[1] = 1;
				} else {
					conditionFlags[1] = 0;
				}
				PC += 2;
				break;

			default:
				status = INS;
				break;
		}
	}
}

/*makes jumps according to opcode and conditional flags*/
void execute_jXX(unsigned char * byte){
	int *destination = (int *)(memoryArray+PC+1);
	check_address(destination);
	switch(*byte) {
		case 0x70:
			PC = *destination;
			break;
		case 0x71:
			if(((conditionFlags[1] ^ conditionFlags[2])| conditionFlags[0]) == 1)
				PC = *destination;
			else
				PC += 5;
			break;
		case 0x72:
			if((conditionFlags[1] ^ conditionFlags[2]) == 1)
				PC = *destination;
			else
				PC += 5;
			break;
		case 0x73:
			if(conditionFlags[0] == 1) {
				PC = *destination;
				
			}else{ 
				PC += 5;
			}
			break;
		case 0x74:
			if(conditionFlags[0] == 0)
				PC = *destination;
			else 
				PC += 5;
			break;
		case 0x75:
			if((conditionFlags[1] ^ conditionFlags[2]) == 0)
				PC =*destination;
			else 
				PC += 5;
			break;
		case 0x76:
			if(((conditionFlags[1] ^ conditionFlags[2]) & conditionFlags[0]) == 0)
				PC =*destination;
			else 
				PC += 5;
			break;
		default:
			status = INS;
			break;
	}
}
/*executes call
decrements stack pointer by 4
writes incremented PC to new value stack pointer*/
void execute_call(unsigned char * byte){

	int *valC = (int *)(memoryArray + PC + 1);
	check_address(valC);
	int valP = PC+5;
	int valB = registers[4];
	int valE = valB - 4;
	int *mem = (int*)(memoryArray + valE);
	*mem = valP;
	registers[4] = valE;
	PC = *valC;


}
/*reads stack pointer and rA
decrements stackpointer by 4 
stores value at new stack pointer
and updates stack pointer*/
void execute_pushl(unsigned char * byte){
	check_rA();
	check_F();

	rA = getRegisterA(byte);
	int valA = registers[rA];
	int valB = registers[4];
	int valE = valB - 4;
	int *valM = (int*)(memoryArray+valE);
	*valM = valA;
	registers[4] = valE;
	PC += 2;
}
/*increments stack pointer 
reads return address
updates stack pointer*/
void execute_return(unsigned char * byte){
	int valA = registers[4];
	int valB = registers[4];
	int valE = valB + 4;
	int *valM = (int*)(memoryArray+valA);
	check_address(valM);
	registers[4] = valE;
	PC = *valM;
}
/*increments stack pointer by 4
reads from old stack pointer
updates stack pointer and writes to register*/
void execute_popl(unsigned char * byte){
	check_rA();
	check_F();
	rA = getRegisterA(byte);
	int valA = registers[4];
	int valB = registers[4];
	int valE = valB + 4;
	int *valM = (int*)(memoryArray + valA);
	registers[4] = valE;
	registers[rA] = *valM;
	PC += 2;
}
/*executes readb and readl according to opcode*/
void execute_readX(unsigned char * byte){
	check_rA();
	check_F();

	rA = getRegisterA(byte);
	int *displacement = (int *)(memoryArray+PC+2);

	unsigned char inputByte;
	int inputLong;

	switch (*byte) {
		case 0xc0:
			if (status == AOK ) {
				conditionFlags[1] = 0;
				conditionFlags[2] = 0;
				if(scanf("%c", &inputByte)	==	EOF){
					conditionFlags[0] = 1;
				} else {
					unsigned char *readByte = (unsigned char*)(memoryArray + registers[rA] + *displacement);
					*readByte = inputByte;
					conditionFlags[0] = 0;
				}
				PC += 6;
			}
			break;
		case 0xc1:
			if (status == AOK) {
				conditionFlags[1] = 0;
				conditionFlags[2] = 0;
				if(scanf("%d", &inputLong)==EOF){
					conditionFlags[0] = 1;
				} else {
					int *readLong = (int*)(memoryArray + registers[rA] + *displacement);
					*readLong = inputLong;
					conditionFlags[0] = 0;
				}
				PC += 6;
			}
			break;
	
	} 
	

}
/*executes writeb and writel according to opcode*/
void execute_writeX(unsigned char * byte){
	check_rA();
	check_F();
	rA = getRegisterA(byte);
	unsigned char *writeByte;
	int *writeLong;
	int *displacement = (int *)(memoryArray + PC + 2);
	

	switch (*byte) {
		case 0xd0:
			if (status == AOK) {
				writeByte = (unsigned char*)(memoryArray + registers[rA] + *displacement);
				printf("%c", *writeByte);
				PC += 6;
			}
			break;
		case 0xd1:
			if (status == AOK) {
				writeLong = (int*)(memoryArray + registers[rA] + *displacement);
				printf("%d", *writeLong);
				PC += 6;
			}
			break;
		default:
			status = INS;
			break;
	}

}
/*moves byte to long*/
void execute_movsbl(unsigned char * byte){
	if(status == AOK) {
		rA = getRegisterA(byte);
		rB = getRegisterB(byte);
		int *displacement = (int *)(memoryArray+PC+2);
		int destination = registers[rB];
		registers[rA] = memoryArray[registers[rB] + *displacement];
	}
	PC += 6;
}
/*reads opcode*/
void decode(unsigned char * byte) {

	
	switch (*byte) {
		case 0x00:
			PC++;
			break;
		case 0x10:
			status = HLT;
			break;
		case 0x20:
			execute_rrmovl(byte);
			break;
		case 0x30:
			execute_irmovl(byte);	
			break;
		case 0x40:
			execute_rmmovl(byte);
			break;
		case 0x50:
			execute_mrmovl(byte);
			break;
		case 0x60:
			execute_OP1(byte);
			break;
		case 0x61:
			execute_OP1(byte);
			break;
		case 0x62:
			execute_OP1(byte);
			break;
		case 0x63:
			execute_OP1(byte);
			break;
		case 0x64:
			execute_OP1(byte);
			break;
		case 0x65:
			execute_OP1(byte);
			break;
		case 0x70:
			execute_jXX(byte);
			break;
		case 0x71:
			execute_jXX(byte);
			break;
		case 0x72:
			execute_jXX(byte);
			break;
		case 0x73:
			execute_jXX(byte);
			break;
		case 0x74:
			execute_jXX(byte);
			break;
		case 0x75:
			execute_jXX(byte);
			break;
		case 0x76:
			execute_jXX(byte);
			break;
		case 0x80:
			execute_call(byte);
			break;
		case 0x90:
			execute_return(byte);
			break;
		case 0xa0:
			execute_pushl(byte);
			break;
		case 0xb0:
			execute_popl(byte);
			break;
		case 0xc0:
			execute_readX(byte);
			break;
		case 0xc1:
			execute_readX(byte);
			break;
		case 0xd0:
			execute_writeX(byte);
			break;
		case 0xd1:
			execute_writeX(byte);
			break;
		case 0xe0:
			execute_movsbl(byte);
			break;
		default:
			status = INS;
			break;
			fprintf(stderr, "ERROR, invalid instruction detected. \n" );
			exit(-1); 
	}
}

/*fetches instructions and sends to decode*/
void readInstructions(char * memory) {
	while(status == AOK ) {
		unsigned char *byte = (unsigned char *)(memory+PC);
		decode(byte);
	}
	if (status == ADR) {
		fprintf(stderr, "ERROR, invalid address detected. \n" );
		exit(-1);
	} else if (status == INS) {
		fprintf(stderr, "ERROR, invalid instruction detected. \n" );
		exit(-1);
	} else {
		exit(0); 
	} 
	
} 


/*take .y86 file and reads lines
allocates memory accodring to .size
and fills in the bytes and chars according to the .y86 file*/
void writeToMemory(FILE * inputfile) { 

    int curr;
    unsigned int loc;
    unsigned int data;
    char *token;
    char temp [3];
    unsigned int index;

    status = AOK;

        token = malloc(sizeof(char)*8);
        fscanf(inputfile,"%s\t%x",token,&memorySize);
        memoryArray = (char*) malloc(memorySize * sizeof(char));

        while(fscanf(inputfile, "%s", token)!=EOF){

            if(!strcmp(token, ".text")){
                fscanf(inputfile, "\t %x \t", &index);
                PC = index;
				int textCounter = 0;
                char textDir[1000];
                fgets(textDir, 1000, inputfile);
				int len = strlen(textDir);
				int b = 0;
				while(b < len){
                    temp[0] = textDir[b];
                    b++;
                    temp[1] = textDir[b];
					b++;
                    temp[2] = '\0';
                    sscanf(temp, "%x", &data);
                    memoryArray[index] = data;
                    index++;
				}
            }
            else if((!strcmp(token,".byte")) || (!strcmp(token,".long"))) {
                fscanf(inputfile, "\t %x \t %x",&loc, &data);
                memoryArray[loc] = data;
            }
            else if(!strcmp(token,".string")){
                fscanf(inputfile, "\t %x \t",&loc);
                curr = fgetc(inputfile);
                while((curr=getc(inputfile))!='\n'){
                    memoryArray[loc] = curr;
                    loc++;
                }
            }
            
        }
    free(memoryArray);
    free(token);
}


int main(int argc, char **argv){
	if (argc != 2) {
		fprintf(stderr, "ERROR: invalid number of arguments \n");
		return 0;
	} else if(argv[1][0] == '-' && argv[1][1] == 'h'){
		printf("Usage: y86emul [-h] <y86 input file> \n");
		return 0;
	}
	
	FILE * f = fopen(argv[1], "r");
	if(f == NULL){
		fprintf(stderr, "ERROR: empty file.");
		return 0;

	} else {
		writeToMemory(f);
	}
	fclose(f);

	status = AOK;

	readInstructions(memoryArray);

	return 0;
}
