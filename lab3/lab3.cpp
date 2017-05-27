#include <dos.h>
#include <stdio.h>
#include <stdlib.h>

char color = 0x00; 
int show = 1; 

struct richChar
{
  unsigned char c; 
  unsigned char a;
};


void updateStatus(); 
void print(int, int, int); 
void printstr(int, int, int);


void interrupt (*i70) (...);
void interrupt (*i71) (...);
void interrupt (*i72) (...);
void interrupt (*i73) (...);
void interrupt (*i74) (...);
void interrupt (*i75) (...);
void interrupt (*i76) (...);
void interrupt (*i77) (...);

void interrupt (*i8) (...);
void interrupt (*i9) (...);
void interrupt (*iA) (...);
void interrupt (*iB) (...);
void interrupt (*iC) (...);
void interrupt (*iD) (...);
void interrupt (*iE) (...);
void interrupt (*iF) (...);


void interrupt  custom88(...) { 
	if (show == 1) {
		updateStatus(); 
	}
	i8(); 
}

void interrupt  custom89(...) { 
	if (show == 1) {
		color++; 
		updateStatus(); 
	}	
	i9(); 
}
void interrupt  custom8A(...) { 
	if (show == 1) {
		updateStatus(); 
	}
	iA(); 
}
void interrupt  custom8B(...) { 
	if (show == 1) {
		updateStatus(); 
	}
	iB(); 
}
void interrupt  custom8C(...) { 
	if (show == 1) {
		updateStatus(); 
	} 
	iC(); 
}
void interrupt  custom8D(...) { 
	if (show == 1) {
		updateStatus(); 
	}
	iD(); 
}
void interrupt  custom8E(...) { 
	if (show == 1) {
		updateStatus(); 
	}
	iE(); 
}
void interrupt  custom8F(...) { 
	if (show == 1) {
		updateStatus(); 
	}
	iF(); 
}

void interrupt  custom8(...) { 
	if (show == 1) {
		updateStatus(); 
	}
	i70(); 
}
void interrupt  custom9(...) { 
	if (show == 1) {
		updateStatus(); 
	} 
	i71(); 
} 
void interrupt  customA(...) { 
	if (show == 1) {
		updateStatus(); 
	} 
	i72(); 
}
void interrupt  customB(...) { 
	if (show == 1) {
		updateStatus(); 
	}
	i73(); 
}
void interrupt  customC(...) { 
	if (show == 1) {
		color++;
		updateStatus(); 
	}
	i74(); 
} 
void interrupt  customD(...) { 
	if (show == 1) {
		updateStatus(); 
	} 
	i75(); 
}
void interrupt  customE(...) { 
	if (show == 1) {
		updateStatus(); 
	} 
	i76(); 
}
void interrupt  customF(...) { 
	if (show == 1) {
		updateStatus(); 
	} 
	i77(); 
}


void byteToString(int byte, char* string, int offset) {
	  short symb;
	  int i;
	  for(i = 0; i < 8; i++) {    
		symb = byte%2;
		*(string+offset+i) = '0'+symb;
		byte /= 2;
      }
}

void updateStatus() {

	char temp;
	int i, j;
	richChar far* videoBufferBegin = (richChar far *)MK_FP(0xB800, 0);
	for (i = 0; i < 80*5; i++) {	// clear 5 rows
		(videoBufferBegin+i)->c = ' ';
		(videoBufferBegin+i)->a = 0;
	}

	char* stateMatrix[5];
	stateMatrix[0] = "+-------+--Master--+--Slave---+";
	stateMatrix[1] = "| Masks | 00000000 | 00000000 |";
	stateMatrix[2] = "|Service| 00000000 | 00000000 |";
	stateMatrix[3] = "| State | 00000000 | 00000000 |";
	stateMatrix[4] = "+-------+----------+----------+";

	byteToString(inp(0x21), stateMatrix[1], 10);
	outp(0x20, 0x0B);
	byteToString(inp(0x20), stateMatrix[2], 10);
	outp(0x20, 0x0A);
	byteToString(inp(0x20), stateMatrix[3], 10);
	
	
	byteToString(inp(0xA1), stateMatrix[1], 21);
	outp(0xA0, 0x0B);
	byteToString(inp(0xA0), stateMatrix[2], 21);
	outp(0xA0, 0x0A);
	byteToString(inp(0xA0), stateMatrix[3], 21);
	
	color = (color < 7) ? color : 0; 
	for (j = 0; j < 31; j++) {
		(videoBufferBegin+j)->c = *(*(stateMatrix)+j);
		(videoBufferBegin+j)->a = color;
	}
	for (j = 0; j < 31; j++) {
		(videoBufferBegin+1*80+j)->c = *(*(stateMatrix+1)+j);
		(videoBufferBegin+1*80+j)->a = color;
	}
	for (j = 0; j < 31; j++) {
		(videoBufferBegin+2*80+j)->c = *(*(stateMatrix+2)+j);
		(videoBufferBegin+2*80+j)->a = color;
	}
	for (j = 0; j < 31; j++) {
		(videoBufferBegin+3*80+j)->c = *(*(stateMatrix+3)+j);
		(videoBufferBegin+3*80+j)->a = color;
	}
	for (j = 0; j < 31; j++) {
		(videoBufferBegin+4*80+j)->c = *(*(stateMatrix+4)+j);
		(videoBufferBegin+4*80+j)->a = color;
	}
}


void init() {
	i8 = getvect(0x08);	// timer
	i9 = getvect(0x09);	// keyboard
	iA = getvect(0x0A);	// 
	iB = getvect(0x0B);	// COM2
	iC = getvect(0x0C);	// COM1
	iD = getvect(0x0D);	// HDD (X edition)
	iE = getvect(0x0E);	// floppy disk
	iF = getvect(0x0F);	// printer

	i70 = getvect(0x70); // RTC
	i71 = getvect(0x71); // EGA controller
	i72 = getvect(0x72); // --
	i73 = getvect(0x73); // --
	i74 = getvect(0x74); // --
	i75 = getvect(0x75); // 8087
	i76 = getvect(0x76); // HDD
	i77 = getvect(0x77); // --

	setvect(0x88, custom88); 
	setvect(0x89, custom89);
	setvect(0x8A, custom8A);
	setvect(0x8B, custom8B);
	setvect(0x8C, custom8C);
	setvect(0x8D, custom8D);
	setvect(0x8E, custom8E);
	setvect(0x8F, custom8F);

	setvect(0X08, custom8);
	setvect(0X09, custom9);
	setvect(0X0A, customA);
	setvect(0X0B, customB);
	setvect(0X0C, customC);
	setvect(0X0D, customD);
	setvect(0X0E, customE);
	setvect(0X0F, customF);

	printf("%d", sizeof(i8));

	_disable(); 

	outp(0X20, 0x11);	
	outp(0X21, 0x88);  
	outp(0x21, 0x04); 	
	outp(0X21, 0x01);	

	outp(0xA0, 0x11);	
	outp(0xA1, 0x08);	
	outp(0xA1, 0x02);	
	outp(0xA1, 0x01);       

	_enable(); 
}

int main() {
      unsigned far *fp;
      init();

      FP_SEG (fp) = _psp; 
      FP_OFF (fp) = 0x2c; 
					
      _dos_freemem(*fp);

      _dos_keep(0, (_DS -_CS) + (_SP/16) + 1);
	return 0;
}