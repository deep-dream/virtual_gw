#pragma once
#include <windows.h>

template<class Storage, int count_per_second, int metrics>
struct	diff_precision_time{
	Storage		value;
	diff_precision_time():value(0){}
	diff_precision_time( Storage v ):value(v*count_per_second/metrics){}
	diff_precision_time( Storage v, int fake ):value(v){}
	
	float to_f(){ return double(value*metrics)/count_per_second; }

	template<class T>
	Storage	operator*( const T& v){ return value*v*metrics/count_per_second; }

	template<class T>
	Storage	operator%( const T& v){ return (value*metrics/count_per_second)%v; }
	
	diff_precision_time	operator-( const diff_precision_time& r){ return diff_precision_time( value - r.value, 1 ); }
	
	bool operator >( Storage v ){ return value*metrics/count_per_second > v; }
	bool operator >=( Storage v ){ return value*metrics/count_per_second >= v; }
	bool operator <( Storage v ){ return value*metrics/count_per_second < v; }
};

struct seconds{
	static const int count_per_second = 1;
	typedef int	storage_type;
};

struct milliseconds{
	static const int count_per_second = 1000;
	typedef int	storage_type;
};

struct microseconds{
	static const int count_per_second = 1000000;
	typedef __int64	storage_type;
};

template<int metrics, class P>
class timer{	
	LARGE_INTEGER Frequency;
public:
	typedef	diff_precision_time<typename P::storage_type, P::count_per_second, metrics>		base_type;
	
	timer(){ QueryPerformanceFrequency(&Frequency); }
	
	base_type get_time(){
		LARGE_INTEGER Count;
		QueryPerformanceCounter(&Count);
		return base_type(P::count_per_second * Count.QuadPart / Frequency.QuadPart, 1);
	}
};