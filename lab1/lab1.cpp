#include <iostream>
#include <math.h>
#include <Windows.h>

void main()
{
	double begin = 0, end = 3000, step = 0.1;
	double x, f;
	LARGE_INTEGER freq, beginTick, endTick;

	x = begin;
	f = 0;
	// -----ASM-----
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&beginTick);
	_asm {
			finit		// initializig fpu

			fld end		// right border				end;
			fld x		// current value			end;x
loop_start:
			fcom		// check if out of range
			fstsw ax	// get flags to ax
			and ah, 01000101b	// check if no
			jz loop_end
			
			fld1		//							end;x;1
			fld x		//							end;x;1;x

			fsqrt	// end;x;1;sqrt(x)
			fadd	// end;x;1+sqrt(x)
			fcos	// end;x;cos(1+sqrt(x))
			fsin	// end;x;sin(cos(1+sqrt(x)))

			fld f	// end;x;sin(cos(1+sqrt(x)));f
			fadd	// end;x;sin(cos(1+sqrt(x)))+f
			fstp f	// end;x

			fadd step	// next step
			fst x	// refresh x

			jmp loop_start
loop_end:
			fwait
	}
	
	QueryPerformanceCounter(&endTick);	
	std::cout <<  "ASM result:" << f << std::endl;
	std::cout << "ASM time: " << (endTick.QuadPart - beginTick.QuadPart)*1000.0f/freq.QuadPart << std::endl;
	// -----C-----
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&beginTick);
	for(x = begin, f = 0; x <= end; x+=step )
	{
		f += sin(cos(sqrt(x) + 1));
	}
	QueryPerformanceCounter(&endTick);
	std::cout <<  "C result:"<< f <<std::endl;
	std::cout << "C time: " << (endTick.QuadPart - beginTick.QuadPart)*1000.0f/freq.QuadPart << std::endl;

	system ("pause");
}
