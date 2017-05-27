#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#define ROW_NUM1 700
#define COL_NUM1 50
#define ROW_NUM2 50
#define COL_NUM2 700

struct matrix {
	union matrixBase {
		short* term;
		int* result;
	} base;
	short rows;
	short cols;
	bool isResult;
} m1, m2, result;

void printMatrix(struct matrix);
void clearMatrix(struct matrix);
double calcC();
double calcASM();
double calcMMX();

int main() {	
	if (COL_NUM1 != ROW_NUM2) {
		printf("Illegal matrix size\n");
		return 0;
	}
	double begin = 0, end = 10000, step = 0.1;
	double x, f;
	clock_t freq, beginTick, endTick;

	m1.base.term = (short*)malloc(sizeof(short)*ROW_NUM1*COL_NUM1);
	m1.cols = COL_NUM1;
	m1.rows = ROW_NUM1;
	m1.isResult = false;

	m2.base.term = (short*)malloc(sizeof(short)*ROW_NUM2*COL_NUM2);
	m2.cols = COL_NUM2;
	m2.rows = ROW_NUM2;
	m2.isResult = false;

	result.base.result = (int*)malloc(sizeof(int)*ROW_NUM1*COL_NUM2);
	result.cols = COL_NUM2;
	result.rows = ROW_NUM1;
	result.isResult = true;

	for (int i = 0; i < ROW_NUM1; ++i) {
		for (int j = 0; j < COL_NUM1; ++j) {
			*(m1.base.term+i*COL_NUM1+j) = (i+j)%10;
		}
	}

	for (int i = 0; i < ROW_NUM2; ++i) {
		for (int j = 0; j < COL_NUM2; ++j) {
			*(m2.base.term+i*COL_NUM2+j) = i%10;
		}
	}
	clearMatrix(result);


	//printf("First matrix:\n");
	//printMatrix(m1);
	printf("----------------\n");
	//printf("Second matrix:\n");
	//printMatrix(m2);
	printf("----------------\n");

	double cTime = calcC();
	printf("C time: %f\n", cTime);
	//printMatrix(result);
	printf("----------------\n");

	clearMatrix(result);

	double asmTime = calcASM();
	printf("ASM time: %f\n", asmTime);
	//printMatrix(result);
	printf("----------------\n");

	clearMatrix(result);

	double mmxTime = calcMMX();
	printf("MMX time: %f\n", mmxTime);
	//printMatrix(result);
	printf("----------------\n");

	short* a = m1.base.term;
	free(a);
	m1.base.term = NULL;
	free(m2.base.term);
	free(result.base.result);// = NULL;
	return 0;
}


void printMatrix(struct matrix m) {
	for (int i = 0; i < m.rows; ++i) {
		for (int j = 0; j < m.cols; ++j) {
			if (m.isResult) {
				printf("%d ", *(m.base.result+m.cols*i+j));
			} else {
				printf("%d ", *(m.base.term+m.cols*i+j));
			}
		}
		printf("\n"); 
	}
}
void clearMatrix(struct matrix m) {
	for (int i = 0; i < m.rows; ++i) {
		for (int j = 0; j < m.cols; ++j) {
			if (m.isResult) {
				*(m.base.result+m.cols*i+j) = 0;
			} else {
				*(m.base.term+m.cols*i+j) = 0;
			}			
		}
	}
}

double calcC() {
	LARGE_INTEGER freq, beginTick, endTick;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&beginTick);
	for (int i = 0; i < m1.rows; i++) {	
		for (int j = 0; j < m2.cols; j++) {	
			int data = 0;
			for (int k = 0; k < m1.cols; k++) {	// col of second
				data += *(m1.base.term+i*m1.cols+k) * *(m2.base.term+k*m2.cols+j);
			}
			*(result.base.result+i*result.cols+j) = data; 
		}
	}
	QueryPerformanceCounter(&endTick);
	return (endTick.QuadPart - beginTick.QuadPart)*1000.0f/freq.QuadPart;
}

double calcASM() {
	// �������
	int b = m1.rows*m1.cols*2;	
	int d = m2.cols*2;
	int a1 = m1.cols*2;
	int a2 = m2.rows*m2.cols*2;	

	// ��������
	int bc = 0;	//m1 rows counter , used in loop 1
	int dc = 0;	//m2 cols counter , used in loop 2
	int a1c = 0;	//m1 cols counter, used in loop 3
	int a2c = 0;	//m2 rows counter, used in loop 3
	int rdc = 0;	//result cols counter
	int rbc = 0;	//result rows counter

	// ������
	int bci = m1.cols*2;	//m1 rows counter 
	int dci = 2;	//m2 cols counter
	int a1ci = 2;	//m1 cols counter
	int a2ci = m2.cols*2;	//m2 rows counter	

	int accumulator = 0;

	LARGE_INTEGER freq, beginTick, endTick;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&beginTick);
	_asm {
		pusha
loop_3:
		
loop_2:
		
loop_1:
		// 1 -- ������� ����� ������������ (������ �� �������), ��������� � �����������
		// ������� �������� �� ������� m1Base + bc + a1c; m2Base + dc + a2c
		// 
		// �������� ������� ������ ������� �� �������� � ax
		// �������� ������� ������ ������� �� �������� 
		// �����������
		// ��������� � accumulator
		// ��������� a1c, a2c �� a1ci, a2ci
		mov ebx, m1.base.term
		add ebx, bc
		add ebx, a1c // ebx -- ����� �������� �� m1
		mov ax, word ptr [ebx]	
		mov ebx, m2.base.term
		add ebx, dc
		add ebx, a2c // ebx -- ����� �������� �� m2
		imul word ptr [ebx]	// ����� ������������ ���������
		
		add accumulator, eax

		mov ebx, a1ci	// ��������� ����������
		add a1c, ebx
		mov ebx, a2ci
		add a2c, ebx

		mov ebx, a1 // ����� ���������� � �� a2
		cmp a1c, ebx
		jl loop_1

		// 2 -- ������ �� ������ ������ �������: ���������� � ������ �������� ������������ � ������� ���

		// ������� �������� a1c, a2c 
		// ������� ����������� � ���������
		// ����������� dc �� dci		
		// ���������, ���� �� ����� dc �� d
		// ������� �����������

		mov ebx, result.base.result		// ������� ���������
		add ebx, rdc
		add ebx, rbc
		mov eax, accumulator
		mov [dword ptr ebx], eax

		mov a1c, 0				//������� �������� ����� 1 � ������������
		mov a2c, 0
		mov eax, 0
		mov accumulator, eax

		mov eax, dci			//��������� ������� ������� �������������� �������
		mov ecx, 2h
		mul ecx
		add rdc, eax
		

		mov eax, dc				// ��������� ������� �� ������ ������ �������
		add eax, dci
		mov dc, eax
		
		cmp eax, d
		jl loop_2

		// 3 -- �������� �� ������� ������ �������
		//
		// ������� �������� dc � rdc
		// ����������� �������� bc �� bci
		// ���������, ���� �� ����� bc �� b
		mov dc, 0
		mov rdc, 0

		mov eax, a2ci		// ��������� ������� �� ������ ������ �������
		mov ecx, 2h
		mul ecx
		add rbc, eax

		mov eax, bc
		add eax, bci
		mov bc, eax

		cmp eax, b
		jl loop_1

		popa
	}
	QueryPerformanceCounter(&endTick);
	return (endTick.QuadPart - beginTick.QuadPart)*1000.0f/freq.QuadPart;
}

double calcMMX() {
	// �������
	int b = m1.rows*m1.cols*2;	
	int d = m2.cols*2;
	int a1 = m1.cols*2;
	int a2 = m2.rows*m2.cols*2;	

	// ��������
	int bc = 0;	//m1 rows counter , used in loop 1
	int dc = 0;	//m2 cols counter , used in loop 2
	int a1c = 0;	//m1 cols counter, used in loop 3
	int a2c = 0;	//m2 rows counter, used in loop 3
	int rdc = 0;	//result cols counter
	int rbc = 0;	//result rows counter

	// ������
	int bci = m1.cols*2;	//m1 rows counter 
	int dci = 2;	//m2 cols counter
	int a1ci = 2*4;	//m1 cols counter
	int a2ci1 = m2.cols*2; //m2 rows counter for each number to put to the MMX
	int a2ci = m2.cols*2*4;	//m2 rows counter
	int rdci = 4;	//result cols counter
	int rbci = result.cols*4;	//result rows counter

	long long tmp = 0; // MMX ������� ��� �������

	int accumulator = 0;

	LARGE_INTEGER freq, beginTick, endTick;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&beginTick);
	_asm {

		pusha
loop_3:
		
loop_2:
		
loop_1:
		// 1 -- ������� ����� ������������ (������ �� �������), ��������� � �����������
		// ������� �������� �� ������� m1Base + bc + a1c; m2Base + dc + a2c
		// 
		// ���������� 4 �������� � ���� ������� MMX
		// ���������� ������� ������� �� 0 ������� tmp
		// ��������� dc �� dci
		// ���������� ������� ������� �� 2 ������� tmp
		// ��������� dc �� dci
		// ���������� ������� ������� �� 4 ������� tmp
		// ��������� dc �� dci 
		// ���������� ������� ������� �� 6 ������� tmp
		// ������� tmp �� ������ ������� MMX
		// ����������� �������� MMX
		// ��������� ������� ���� ���������� � ebx (movd)
		// ����� ������ �� 4 �����
		// ��������� ������� ���� ���������� � eax
		// ����������
		// ��������� � accumulator
		// ��������� a1c, a2c �� a1ci, a2ci
		mov ebx, m1.base.term
		add ebx, bc
		add ebx, a1c // ebx -- ����� �������� �� m1

		lea esi, tmp
		mov eax, [ebx]	// ????? ����� �����? mov tmp, qword ptr [ebx]
		mov [esi], eax
		mov eax, [ebx+4]
		mov [esi+4], eax

		movq MM0, qword ptr [esi]
		mov ebx, m2.base.term
		add ebx, dc
		add ebx, a2c // ebx -- ����� �������� �� m2
		mov ax, word ptr [ebx]	// ������� 1-��
		lea esi, tmp
		mov [esi], ax

		add ebx, a2ci1			// ������� 2-��
		mov ax, word ptr [ebx]	
		mov [esi+2], ax

		add ebx, a2ci1			// ������� 3-��
		mov ax, word ptr [ebx]	
		mov [esi+4], ax

		add ebx, a2ci1			// ������� 4-��
		mov ax, word ptr [ebx]	
		mov [esi+6], ax

		movq MM1, tmp

		pmaddwd MM1, MM0

		movd eax, MM1

		psrlq MM1, 32

		movd ebx, MM1

		emms

		add eax, ebx	// ���������		
		add accumulator, eax

		mov ebx, a1ci	// ��������� ����������
		add a1c, ebx
		mov ebx, a2ci
		add a2c, ebx

		mov ebx, a1 // ����� ���������� � �� a2
		cmp a1c, ebx
		jl loop_1

		// 2 -- ������ �� ������ ������ �������: ���������� � ������ �������� ������������ � ������� ���

		// ������� �������� a1c, a2c 
		// ������� ����������� � ���������
		// ����������� dc �� dci		
		// ���������, ���� �� ����� dc �� d
		// ������� �����������

		mov ebx, result.base.result		// ������� ���������
		add ebx, rdc
		add ebx, rbc
		mov eax, accumulator
		mov [dword ptr ebx], eax

		mov a1c, 0				//������� �������� ����� 1 � ������������
		mov a2c, 0
		mov eax, 0
		mov accumulator, eax

		mov eax, rdci			//��������� ������� ������� �������������� �������
		add rdc, eax
		

		mov eax, dc				// ��������� ������� �� ������ ������ �������
		add eax, dci
		mov dc, eax
		
		cmp eax, d
		jl loop_2

		// 3 -- �������� �� ������� ������ �������
		//
		// ������� �������� dc � rdc
		// ����������� �������� bc �� bci
		// ���������, ���� �� ����� bc �� b
		mov dc, 0
		mov rdc, 0

		mov eax, rbci		// ��������� ������� �� ������ ������ �������
		add rbc, eax

		mov eax, bc
		add eax, bci
		mov bc, eax

		cmp eax, b
		jl loop_1
		
		popa
	}
	tmp = 0;
	QueryPerformanceCounter(&endTick);
	double time = (endTick.QuadPart - beginTick.QuadPart)*1000.0f/freq.QuadPart;
	return time;
}