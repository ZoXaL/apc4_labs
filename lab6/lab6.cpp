#include <dos.h>
#include <stdio.h>
#include <io.h>
#include <stdlib.h>

#define BB_RECEIVED 0
#define BB_NEED_REPEAT 1
#define BB_NOT_RECIVED 2

unsigned short int blink_byte_status = BB_NOT_RECIVED;
void interrupt(*int_09_old)(...);

enum keyboad_indicator {
	SCROLL_LOCK = 0x01, NUM_LOCK = 0x02, CAPS_LOCK = 0x04
};

void cleanup() {
	disable();
	setvect(0x09, int_09_old);
	enable();
}

void interrupt int_09_new(...) {
	unsigned char scan_code;
	scan_code = inp(0x60);

	if (scan_code == 0xFE) {
		blink_byte_status = BB_NEED_REPEAT;
		outp(0x20, 0x20);
		outp(0xA0, 0x20);
		return;
	}

	if (scan_code == 0xED 
		|| scan_code == SCROLL_LOCK
		|| scan_code == NUM_LOCK
		|| scan_code == CAPS_LOCK
		) {
		blink_byte_status = BB_RECEIVED;
		printf("int_09 got code: %x\n", scan_code);
		outp(0x20, 0x20);
		outp(0xA0, 0x20);
		return;
	}
}

void light(enum keyboad_indicator indicator_code) {
	int timeout = 50000;
	int attempt_count = 10;
	do {
		while (timeout-- > 0 || (inp(0x64) & 0x02) != 0) {};
		if (timeout == 0) {
			printf("Light: timeout error (can not write command code, buffer is not empty)\n");		
			cleanup();
			exit(1);
		}
		outp(0x60, 0xED); 
		while (blink_byte_status == BB_RECEIVED) {delay(5);}
	} while(attempt_count-- > 0 || blink_byte_status == BB_NEED_REPEAT);
	if (attempt_count == 0) {
		printf("Light: attempt limit exceed (int09 can not get command code)");
		cleanup();
		exit(1);
	}

	timeout = 50000;
	attempt_count = 10;
	do {
		while (timeout-- > 0 || (inp(0x64) & 0x02) != 0) {};
		if (timeout == 0) {
			printf("Light: timeout error (can not write command, buffer is not empty)\n");		
			cleanup();
			exit(1);
		}
		outp(0x60, indicator_code);
		while (blink_byte_status == BB_RECEIVED) {delay(5);}
	} while(attempt_count-- > 0 || blink_byte_status == BB_NEED_REPEAT);
	if (attempt_count == 0) {
		printf("Light: attempt limit exceed (int09 can not get command)");
		cleanup();
		exit(1);
	}	
}

void unlight(enum keyboad_indicator indicator_code) {
	light(indicator_code ^ 0x07);
}

int main() {
	disable();
	int_09_old = getvect(0x09);
	setvect(0x09, int_09_new);
	enable();

	light(SCROLL_LOCK | NUM_LOCK | CAPS_LOCK);		// all on
	delay(1000);

	unlight(SCROLL_LOCK | NUM_LOCK | CAPS_LOCK);	// all off
	delay(1000);

	light(SCROLL_LOCK | NUM_LOCK | CAPS_LOCK);		// all on
	delay(1000);

	unlight(SCROLL_LOCK | NUM_LOCK | CAPS_LOCK);	// all off
	delay(1000);

	light(SCROLL_LOCK | NUM_LOCK | CAPS_LOCK);		// all on
	delay(1000);

	unlight(SCROLL_LOCK | NUM_LOCK | CAPS_LOCK);	// all off
	delay(1000);

	light(SCROLL_LOCK);								// one by one
	delay(1000);
	light(NUM_LOCK);
	delay(1000);
	light(CAPS_LOCK);
	delay(1000);

	unlight(SCROLL_LOCK | NUM_LOCK | CAPS_LOCK);	// all off
	
	disable();
	setvect(0x09, int_09_old);
	enable();
	return 0;
}