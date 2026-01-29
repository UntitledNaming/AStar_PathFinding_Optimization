
#define PROFILE
#include <iostream>
#include <windows.h>
#include <process.h>
#include "ProfilerTLS.h"
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winmm.lib")

unsigned int __stdcall WorkerThread1(LPVOID lpParam);
unsigned int __stdcall WorkerThread2(LPVOID lpParam);
unsigned int __stdcall WorkerThread3(LPVOID lpParam);

void Func1();
void Func2();
void Func3();
void Func4();
void Func5();


int main()
{
	timeBeginPeriod(1);


	CProfilerManager::GetInstance()->Init();

	//스레드 생성
	HANDLE hThread[3];

	hThread[0] = (HANDLE)_beginthreadex(NULL, 0, WorkerThread1, NULL, 0, NULL);
	hThread[1] = (HANDLE)_beginthreadex(NULL, 0, WorkerThread2, NULL, 0, NULL);
	hThread[2] = (HANDLE)_beginthreadex(NULL, 0, WorkerThread3, NULL, 0, NULL);


	while (1)  
	{
		//Space 키 눌렸으면
		if (GetAsyncKeyState(VK_SPACE) & 0x8001)
		{
			CProfilerManager::GetInstance()->ProfileDataOutText(const_cast<WCHAR*>(L"Test"));
		}
	}
}

unsigned int __stdcall WorkerThread1(LPVOID lpParam)
{
	wprintf(L"Thread 1 Start...\n");

	CProfilerManager::GetInstance()->ProfileInit();

	while (1)
	{
		Func1(); 
		Func2();
		Func3();
	}

	wprintf(L"Sleep Go \n");
	Sleep(INFINITE);

	return 0;
}

unsigned int __stdcall WorkerThread2(LPVOID lpParam)
{
	wprintf(L"Thread 2 Start...\n");
	CProfilerManager::GetInstance()->ProfileInit();

	while (1)
	{
		Func2();
		Func3();
	}


	wprintf(L"Sleep Go \n");
	Sleep(INFINITE);



	return 0;
}

unsigned int __stdcall WorkerThread3(LPVOID lpParam)
{
	wprintf(L"Thread 3 Start...\n");
	CProfilerManager::GetInstance()->ProfileInit();

	while (1)
	{
		Func3();
		Func4();
		Func5();
	}


	Sleep(INFINITE);
	return 0;
}

void Func1()
{
	CProfiler p(L"Func1");
	Sleep(5);

}

void Func2()
{
	CProfiler p(L"Func2");
	Sleep(2);
}

void Func3()
{
	CProfiler p(L"Func3");
	Sleep(3);
}

void Func4()
{
	CProfiler p(L"Func4");
	Sleep(4);
}

void Func5()
{
	CProfiler p(L"Func5");
	Sleep(5);
}


