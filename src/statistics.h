#pragma once
#include "nqueue.h"
#include "timer2.h"

struct tr_counter{
	Timer::base_type	last_time;
	Timer::base_type	frame_time;
	int					value;
	int					tmp_value;
	
	tr_counter(Timer::base_type f_time = 500):frame_time(f_time),value(0),tmp_value(0),last_time(0){}

	void add(Timer::base_type time, int size){
		if(!last_time) last_time = time;

		if( time - last_time < frame_time ){
			tmp_value += size;
		}else{
			last_time = time - (time - last_time)%frame_time;
			value = tmp_value;
			tmp_value = size;
		}
	}
};

struct statistics{
	tr_counter	in_counter;
	tr_counter	out_counter;
	tr_counter	in_packets;
	tr_counter	out_packets;
	tr_counter	packets_lost;
	tr_counter	packets_delay;

	void packet_lost(Timer::base_type time, const std::vector<char>& data){
		packets_lost.add( time, 1);
	}
	void packet_in(Timer::base_type time, const std::vector<char>& data){
		in_packets.add( time, 1);
		in_counter.add( time, (int)data.size() - 14 );
	}
	void packet_out(Timer::base_type out_time, Timer::base_type in_time, const std::vector<char>& data ){
		out_packets.add( out_time, 1);
		out_counter.add( out_time, (int)data.size() - 14 );
		packets_delay.add( out_time, out_time-in_time );
	}
};