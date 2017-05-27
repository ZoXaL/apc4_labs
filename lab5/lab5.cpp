#include <dos.h>
#include <conio.h>
#include <stdio.h>

int alarm_set = 0;
void interrupt(*i4A_alarm) (...);
void interrupt i4A_alarm_new(...) {
	for (int i = 0; i < 3; i++) {
		sound(800);
		delay(120);
		nosound();
		delay(120);
		sound(800);
		delay(150);
		nosound();
		delay(600);
	}
	i4A_alarm();

}

long int delay_left = 0;
void interrupt far (*i70)(...);
void interrupt far i70_new(...){
   outp(0x70,0x0C);
   unsigned int cause = inp(0x71);
   if (cause & 0x40) {
		delay_left--;
   }
   i70();

}

void show_usage() {
	printf("0) show current time\n");
	printf("1) set current time\n");
	printf("2) set alarm time\n");
	printf("3) reser alarm time\n");
	printf("4) emulate delay\n");
	printf("5) exit\n");
}

void set_binary_format() {
	for (int i = 0; i < 777; i++) {
		outp(0x70, 0xA);
		if (!(inp(0x71) & 0x80)) {
			delay(10);
			continue;
		}
		outp(0x70, 0x0B);
		outp(0x71, inp(0x71) | 0x80);

		outp(0x70, 0x0B);
		outp(0x71, inp(0x71) | 0x04);

		outp(0x70, 0x0B);
		outp(0x71, inp(0x71) & 0x7F);
		return;
	}
}


void show_time() {
	printf("current time: ");
	outp(0x70, 0x00);
	int seconds = inp(0x71);
	outp(0x70, 0x02);
	int minutes = inp(0x71);
	outp(0x70, 0x04);
	int hours = inp(0x71);

	outp(0x70, 0x06);
	int week_day = inp(0x71);
	outp(0x70, 0x07);
	int month_day = inp(0x71);
	outp(0x70, 0x08);
	int month = inp(0x71);
	outp(0x70, 0x09);
	long int year = inp(0x71);
	printf("%02x:%02x:%02x\t", hours, minutes, seconds);
	printf("%02x/%02x/%02x", month_day, month, year);

	switch(week_day) {
		case 1:
			printf(", %s", "Sun\n");
			break;
		case 2:
			printf(", %s", "Mon\n");
			break;
		case 3:
			printf(", %s", "Tue\n");
			break;
		case 4:
			printf(", %s", "Wed\n");
			break;
		case 5:
			printf(", %s", "Thu\n");
			break;
		case 6:
			printf(" %s", "Fri\n");
			break;
		case 7:
			printf(" %s", "Sat\n");
			break;
	}
}

inline int int2bsd(int int_to_convert) {
	int high_byte = int_to_convert/10;
	int low_byte = int_to_convert%10;
	return (high_byte*16) + low_byte;
}

void set_time() {
	printf("enter hours: ");
	int hours;
	scanf("%d", &hours);
	if (hours > 24 || hours < 0) {
		printf("invalid input\n");
	}

	printf("enter minutes: ");
	int minutes;
	scanf("%d", &minutes);
	if (minutes > 59 || minutes < 0) {
		printf("invalid input\n");
	}
	printf("enter seconds: ");
	int seconds;
	scanf("%d", &seconds);
	if (seconds > 59 || seconds < 0) {
		printf("invalid input\n");
	}

	for (int i = 0; i < 777; i++) {
		outp(0x70, 0xA);
		if (!(inp(0x71) & 0x80)) {
			delay(10);
			//printf();
			continue;
		}
		outp(0x70, 0x0B);
		outp(0x71, inp(0x71) | 0x80);

		// ----- setting time -----
		outp(0x70, 0x04);
		outp(0x71, int2bsd(hours));

		outp(0x70, 0x02);
		outp(0x71, int2bsd(minutes));

		outp(0x70, 0x00);
		outp(0x71, int2bsd(seconds));
		// ------------------------

		outp(0x70, 0x0B);
		outp(0x71, inp(0x71) & 0x7F);
		break;
	}
}


void emulate_delay(long int milisec) {
	printf("emulating delay ...\n");
	delay_left = milisec;

	_disable();
	i70 = getvect(0x70);
	setvect(0x70, i70_new);


	int slave_mask = inp(0xA1);
	outp(0xA1, slave_mask & 0xFE);
	//slave_mask = inp(0xA1);
	//printf("current slave mask: %x\n", slave_mask);

	outp(0x70, 0x0B);
	int rtc_0B = inp(0x71);
	outp(0x71, rtc_0B | 0x40);
	outp(0x70, 0x0B);
	//rtc_0B = inp(0x71);
	//printf("rtc_0B: %x", rtc_0B);

	_enable();

	for(;delay_left > 0;) {
	 //printf("delay left: %ld\n", delay_left);
	}
	_disable();
	outp(0xA1, slave_mask);
	outp(0x71, rtc_0B);
	setvect(0x70, i70);
	_enable();

	return;

}

void set_alarm() {
	printf("set alarm\n");

	printf("enter hours: ");
	int hours;
	scanf("%d", &hours);
	if (hours > 24 || hours < 0) {
		printf("invalid input\n");
		return;
	}

	printf("enter minutes: ");
	int minutes;
	scanf("%d", &minutes);
	if (minutes > 59 || minutes < 0) {
		printf("invalid input\n");
		return;
	}
	printf("enter seconds: ");
	int seconds;
	scanf("%d", &seconds);
	if (seconds > 59 || seconds < 0) {
		printf("invalid input\n");
		return;
	}

	if (alarm_set == 0) {
		i4A_alarm = getvect(0x4A);

		_disable();
		setvect(0x4A, i4A_alarm_new);

		int slave_mask = inp(0xA1);
		outp(0xA1, slave_mask & 0xFE);
		_enable();

		alarm_set = 1;
	}
	//slave_mask = inp(0xA1);
	//printf("current slave mask: %x\n", slave_mask);

	outp(0x70, 0x0B);
	int rtc_0B = inp(0x71);
	outp(0x71, rtc_0B | 0x20);
	outp(0x70, 0x0B);

	outp(0x70, 0x01);
	outp(0x71, int2bsd(seconds));

	outp(0x70, 0x03);
	outp(0x71, int2bsd(minutes));

	outp(0x70, 0x05);
	outp(0x71, int2bsd(hours));

	printf("alarm is set at %d:%d:%d\n", hours, minutes, seconds);


	return;

}

void reset_alarm() {
	if (alarm_set == 1) {
		_disable();
		setvect(0x4A, i4A_alarm);

		int slave_mask = inp(0xA1);
		outp(0xA1, slave_mask | 0x01);
		_enable();

		alarm_set = 0;
		printf("alarm reset\n");
	} else {
		printf("there is no set alarm yet\n");
	}
	outp(0x70, 0x0B);
	int rtc_0B = inp(0x71);
	outp(0x71, rtc_0B & 0xDF);
	return;
}

int main() {
	set_binary_format();
	show_usage();
	printf("select operation: \n");
	char input = getchar();
	while(1) {
		switch(input) {
			case '0': {
				show_time();
				break;
			}
			case '1': {
				set_time();
				break;
			}
			case '2': {
				set_alarm();
				break;
			}
			case '3': {
				reset_alarm();
				break;
			}
			case '4': {
				printf("set delay in milisec: ");
				long int delay;
				scanf("%ld", &delay);
				emulate_delay(delay);
				break;
			}
			case '5':
				printf("bye\n");
				return 0;
			case 10: {
				input = getchar();
				continue;
			}
			case 13: {
				input = getchar();
				continue;
			}
			default :{
				printf("Invalid input\n");
				break;
			}
		}
		show_usage();
		printf("\nselect operation: \n");
		input = getchar();
	}
}