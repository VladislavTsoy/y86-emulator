#ifndef _Y86EMUL_H_
#define _Y86EMUL_H_


int main(int argc, char **argv);
void check_rA();
void check_rB();
void check_F();
void check_address(int *address);
int getRegisterA(unsigned char * byte);
int getRegisterB(unsigned char* byte);
void execute_rrmovl(unsigned char * byte);
void execute_irmovl(unsigned char * byte);
void execute_rmmovl(unsigned char * byte);
void execute_mrmovl(unsigned char * byte);
void execute_OP1(unsigned char * byte);
void execute_jXX(unsigned char * byte);
void execute_call(unsigned char * byte);
void execute_pushl(unsigned char * byte);
void execute_return(unsigned char * byte);
void execute_popl(unsigned char * byte);
void execute_readX(unsigned char * byte);
void execute_writeX(unsigned char * byte);
void execute_movsbl(unsigned char * byte);
void decode(unsigned char * byte) ;
void readInstructions(char * memory);
void writeToMemory(FILE * file);

#endif