#pragma once
#include <windows.h>

class Timer{	
	LARGE_INTEGER Frequency;
public:
	typedef	int base_type;
	
	Timer(){ QueryPerformanceFrequency(&Frequency); }
	
	base_type get_time(){
		LARGE_INTEGER Count;
		QueryPerformanceCounter(&Count);
		return base_type(1000 * Count.QuadPart / Frequency.QuadPart);
	}
};
