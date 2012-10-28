#pragma once
#include "timer2.h"

struct npacket{
	static	int				uid_base;
	int						uid;
	Timer::base_type		in_time;
	std::vector<char>		data;
	npacket(Timer::base_type time, const std::vector<char>& _data):in_time(time),data(_data),uid(++uid_base){}
};	
