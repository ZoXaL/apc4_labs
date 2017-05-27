#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

#define base  1193180
#define SHORT_MAX  65535

enum NOTE {
	EL = 165,
	FL = 174,
	FFL = 185,
	GL = 196,
	GGL = 207,
	AL = 220,
	AAL = 233,
	BL = 247,
	C = 261,
	CC = 277,
	D = 293,
	DD = 311,
	E = 329,
	F = 349,
	FF = 370,
	G = 392,
	GG = 415,
	A = 440,
	AA = 466,
	B = 493,
	CH = 523,
	CCH = 554,
	DH = 587,
	DDH = 622,
	EH = 659,
	FH = 698,
	FFH = 739,
	GH = 784,
	GGH = 830,
	AH = 880,
	AAH = 932,
	BH = 988
};

enum DURATION {
	ONE_SIXTEENTH = 60,
	ONE_EIGHT = 120,
	ONE_QUATER = 240
};

void printByte(char byte);

void printChanelStatus(char chanel) {
	char controlByte = 0xE0;		//11_1_0_000_0
	char chanelPort = 0x40;
	switch (chanel) {
		case 0 : {
			controlByte = 0xE2;		//11_1_0_001_0
			chanelPort = 0x40;	
			break;
		}
		case 1 : {
			controlByte = 0xE4;		//11_1_0_010_0
			chanelPort = 0x41;
			break;
		}
		case 2 : {
			controlByte = 0xE8;		//11_1_0_100_0
			chanelPort = 0x42;
			break;
		}
		default : {
			printf("ERROR: Illegal printChanelStatus usage (ivalid chanel number %d)\n", chanel);
			return;
		}
	}	
	outp(0x43, controlByte);
	char state = inp(chanelPort);
	printf("The \t%d-th chanel state: ", chanel);
	printByte(state);
	printf("\n");
}

void printByte(char byte) {
	char i;
	char byteBuf[8];
	for (i = 7; i >= 0; i--) {
		byteBuf[i] = (byte % 2) ? '1' : '0';
		byte /= 2;
	}
	for (i = 0; i < 8; i++) {
		printf("%c", byteBuf[i]);
	}
}

void printChanelKD(short chanel) {
	char controlByte;
	char chanelPort;
	switch (chanel) {
		case 0 : {
			controlByte = 0xD0;		//11_0_1_001_0 
			chanelPort = 0x40;
			break;
		}
		case 1 : {
			controlByte = 0xD0;		//01_00_000_0 | 11_0_1_010_0
			chanelPort = 0x41;
			break;
		}
		case 2 : {
			controlByte = 0xD0;		//10_00_000_0	| 11_0_1_100_0
			chanelPort = 0x42;
			break;
		}
		default : {
			printf("ERROR: Illegal printChanelKD usage (ivalid chanel number %d)\n", chanel);
			return;
		}
	}	
	unsigned short int kd = 0, kdHigh = 0, kdLow = 0, kdMax = 0, i;

	for (i = 0, kdMax = 0; i < SHORT_MAX; i++) {
		outp(0x43, controlByte); //KK00 0000
		kdLow = inp(chanelPort);
		kdHigh = inp(chanelPort);
		kd = kdHigh * 256 + kdLow;
		if (kd > kdMax)  {
			printf("new kdMax: %hx\n", kd);
			kdMax = kd;
		}
	}
	printf("The %d-th chanel kd: \t%hx\n", chanel, kdMax);
}

// void printChanelKD(short chanel) {
// 	char controlByte;
// 	char chanelPort;
// 	switch (chanel) {
// 		case 0 : {
// 			chanelPort = 0x40;
// 			break;
// 		}
// 		case 1 : {
// 			chanelPort = 0x41;
// 			break;
// 		}
// 		case 2 : {
// 			chanelPort = 0x42;
// 			break;
// 		}
// 		default : {
// 			printf("ERROR: Illegal printChanelKD usage (ivalid chanel number %d)\n", chanel);
// 			return;
// 		}
// 	}	
// 	unsigned short int kd = 0, kdHigh = 0, kdLow = 0, kdMax = 0, i;
	
// 	for (i = 0, kdMax = 0; i < SHORT_MAX; i++) {
// 		kdLow = inp(chanelPort);
// 		kdHigh = inp(chanelPort);
// 		kd = kdHigh * 256 + kdLow;
// 		if (kd > kdMax)  {
// 			kdMax = kd;
// 		}
// 	}
// 	for (i = 0, kdMax = 0; i < SHORT_MAX; i++) {
// 		kdLow = inp(chanelPort);
// 		kdHigh = inp(chanelPort);
// 		kd = kdHigh * 256 + kdLow;
// 		if (kd > kdMax)  {
// 			kdMax = kd;
// 		}
// 	}
// 	printf("The %d-th chanel kd: \t%hx\n", chanel, kdMax);
// }

void random(long maxNumber) {	
	long kdNew = 0, kdLow = 0, kdHigh = 0;
	long prevState = inp(0x61);
	outp(0x61, inp(0x61) & 0xFC | 0x01);	// disable dynamic, enable tick
	
	outp(0x43, 0xB4); // 10_11_010_0
	outp(0x42, maxNumber%256);	// low byte
	outp(0x42, maxNumber/256);	// high byte	

	int i = 0;
	for (i = 0; i < 5; i++) {
		delay(i);	
		kdLow = inp(0x42);
		kdHigh = inp(0x42);
		kdNew = kdHigh * 256 + kdLow;
		printf("%d\n", kdNew);
	}
	outp(0x61, prevState);
}

void playSound(enum NOTE note, enum DURATION duration) {
	int kd;
	outp(0x43, 0xB6);//10_11_011_0

	kd = base / note;

	outp(0x42, kd % 256);
	kd /= 256;
	outp(0x42, kd);

	outp(0x61, inp(0x61) | 0x03);
	delay(duration);
	
	outp(0x61, inp(0x61) & 0xFC);
	// switch pause
	delay(duration/3);
}

void doRock() {
	for (int  i = 0; i < 2; i++) {
		playSound(D, ONE_QUATER);
		playSound(D, ONE_EIGHT);
		playSound(D, ONE_EIGHT);
		playSound(GH, ONE_EIGHT);
		playSound(FFH, ONE_EIGHT);
		playSound(D, ONE_EIGHT);
		playSound(D, ONE_EIGHT);

		playSound(AL, ONE_QUATER);
		playSound(AL, ONE_EIGHT);
		playSound(AL, ONE_EIGHT);
		playSound(EH, ONE_EIGHT);
		playSound(FFH, ONE_EIGHT);
		playSound(AL, ONE_EIGHT);
		playSound(AL, ONE_EIGHT);

		playSound(BL, ONE_QUATER);
		playSound(BL, ONE_EIGHT);
		playSound(BL, ONE_EIGHT);
		playSound(CCH, ONE_EIGHT);
		playSound(BL, ONE_EIGHT);
		playSound(DH, ONE_EIGHT);
		playSound(BL, ONE_EIGHT);

		playSound(FFL, ONE_QUATER);
		playSound(FFL, ONE_EIGHT);
		playSound(FFL, ONE_EIGHT);
		playSound(EL, ONE_EIGHT);
		playSound(EL, ONE_EIGHT);
		playSound(FFL, ONE_EIGHT);
		playSound(FFL, ONE_EIGHT);

		playSound(GL, ONE_QUATER);
		playSound(GL, ONE_EIGHT);
		playSound(GL, ONE_EIGHT);
		playSound(GL, ONE_EIGHT);
		playSound(G, ONE_QUATER);
		playSound(A, ONE_EIGHT);

		playSound(D, ONE_QUATER);
		playSound(D, ONE_EIGHT);
		playSound(D, ONE_EIGHT);
		playSound(GH, ONE_EIGHT);
		playSound(FFH, ONE_EIGHT);
		playSound(EH, ONE_EIGHT);
		playSound(DH, ONE_EIGHT);

		playSound(GL, ONE_QUATER);
		playSound(GL, ONE_EIGHT);
		playSound(GL, ONE_EIGHT);
		playSound(B, ONE_EIGHT);
		playSound(CCH, ONE_EIGHT);
		playSound(DH, ONE_EIGHT);
		playSound(CCH, ONE_EIGHT);

		playSound(AL, ONE_QUATER);
		playSound(AL, ONE_EIGHT);
		playSound(AL, ONE_EIGHT);
		playSound(CCH, ONE_EIGHT);
		playSound(DH, ONE_EIGHT);
		playSound(EH, ONE_EIGHT);
		playSound(DH, ONE_EIGHT);		
	}
	playSound(DH, ONE_QUATER*4);
}

int main() {	
	// 1) get states
	printChanelStatus(0);
	printChanelStatus(1);
	printChanelStatus(2);

	// 2) get kd
	printChanelKD(0);
	printChanelKD(1);
	printChanelKD(2);

	// 3) print random numbers
	long maxRandom;
	printf("Enter max random number: ");
	scanf("%ld", &maxRandom);
	random(maxRandom);
	
	// 2) get kd
	printChanelKD(0);
	printChanelKD(1);
	printChanelKD(2);

	// 4) play sound
	doRock();

	return 0;
}