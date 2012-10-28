#pragma once
#include <queue>
#include <vector>
#include <string>
#include <iostream>
#include "hl/thread_object.h"
#include <list>
#include <map>
#include "hl/params.h"
#include "hl/smart_ptr.h"
#include "nfilter.h"
#include "print.h"
#include "timer2.h"
#include "statistics.h"

#include "npacket.h"

int npacket::uid_base=0;

template<class T>
class packets_raw_queue{
	std::queue<T>	packets;
	size_t			_total_data_size;
	size_t			total_limit;
	statistics		stat;
public:
	packets_raw_queue(int queue_size):_total_data_size(0),total_limit(queue_size){}
	void set_queue_size(size_t new_size){
		std::cout << "New queue size: " << new_size << "\n";
		total_limit = new_size;
	}
	int total_data_size(){ return _total_data_size; }
	void push(const T& p){
		stat.packet_in(p.in_time, p.data);
		if( _total_data_size > total_limit ){
			stat.packet_lost(p.in_time, p.data);
			return;
		}
		packets.push( p );
		_total_data_size += p.data.size();
	}
	T& front(){ return packets.front(); }
	bool empty(){ return packets.empty(); }
	
	void drop_front(Timer::base_type out_time){
		T p = packets.front();
		stat.packet_lost(p.in_time, p.data);
		pop(out_time);
	}
	
	T pop(Timer::base_type out_time){
		T packet = packets.front();
		stat.packet_out(out_time, packet.in_time, packet.data);
		_total_data_size -= packets.front().data.size();
		packets.pop();		
		return packet;
	}
	void show_stat(Timer::base_type time){
		std::cout << " queue_size=" << _total_data_size;
		std::cout << " queue_pkt=" << packets.size();
		std::cout << " in_pkt=" << stat.in_packets.value*2;
		std::cout << " out_pkt=" << stat.out_packets.value*2;
		std::cout << " in_traf=" << stat.in_counter.value*2;
		std::cout << " out_traf=" << stat.out_counter.value*2;
		std::cout << " lost_pkt=" << stat.packets_lost.value*2;
		if (stat.in_packets.value){
			std::cout << " delay="<< stat.packets_delay.value/stat.in_packets.value;
		}else{
			std::cout << " delay=0";
		}
		std::cout << "\n";

		stat.in_packets.add(time, 0);
		stat.out_packets.add(time, 0);
		stat.in_counter.add(time, 0);
		stat.out_counter.add(time, 0);
		stat.packets_lost.add(time, 0);
		stat.packets_delay.add(time, 0);
	}
};
typedef	packets_raw_queue<npacket> packets_t;
//----------------------------------------------------------------------------------
struct qdisc : public intrusive_counter{
	virtual bool can_pop( Timer::base_type time, packets_t& packets )=0;
	virtual std::string describe()=0;
};
//----------------------------------------------------------------------------------
struct rloss_qdisc : public qdisc{
	int rloss;
	int last_uid;
	rloss_qdisc(const std::string& opts ):last_uid(0){
		params( opts ).read("([0-9]+)", rloss);
	}
	virtual bool can_pop( Timer::base_type time, packets_t& packets ){
		while( !packets.empty() && last_uid != packets.front().uid ){
			last_uid = packets.front().uid;
			if( rand() % 100 < rloss ) packets.drop_front(time);
		}
		return !packets.empty();
	}
	virtual std::string describe(){ return print("rloss %u", rloss); }
};
//----------------------------------------------------------------------------------
class band_qdisc : public qdisc{
	int					rate,	burst,	quota,	max_delay;
	Timer::base_type	last_ts;
public:
	band_qdisc(const std::string& opts): quota(0),burst(1500),max_delay(0){
		params( opts ).read("rate ([0-9]+)", rate);
		params( opts ).read("burst +([0-9]+)", burst,false);
		params( opts ).read("max_delay +([0-9]+)", max_delay,false);
	}

	virtual bool can_pop( Timer::base_type time, packets_t& packets ){
		if (rate <= 0) return true;

		Timer::base_type delta_t = time - last_ts;
		int delta_traffic = abs(delta_t * rate /1000);
		
		// do not collect traffic for more than burst bytes
		quota = min( burst, quota + delta_traffic);
		last_ts = time;

		//drop over-delayed packets
		while( max_delay && !packets.empty() && time - packets.front().in_time > max_delay ){
			packets.drop_front(time);
		}

		if( !packets.empty() && quota > 0 ){
			quota -= (int)(packets.front().data.size()-14); //ethernet
			return true;
		}	
		return false;
	}
	virtual std::string describe(){ return print("rate %u, burst %u, max_delay %u", rate, burst, max_delay); }
};
//----------------------------------------------------------------------------------
class delay_qdisc : public qdisc{
	int		delay; //ms
public:
	delay_qdisc(const std::string& opts ){
		params( opts ).read("([0-9]+)", delay);
	}
	virtual bool can_pop( Timer::base_type time, packets_t& packets ){
		return packets.empty() && time - packets.front().in_time >= delay;
	}
	virtual std::string describe(){ return print("delay %u", delay); }
};
//----------------------------------------------------------------------------------
qdisc* Create_qdisc(std::string &qdisc_type, std::string &params){
	if (qdisc_type=="band") return new band_qdisc(params);
	if (qdisc_type=="delay") return new delay_qdisc(params);
	if (qdisc_type=="rloss") return new rloss_qdisc(params);
	throw std::exception();
}
//----------------------------------------------------------------------------------
class nqueue{
	packets_t		packets;
	std::vector< smart_ptr<qdisc> >	qdisc_vec;
public:	
	nfilter			filter;
public:	
	void set(std::string &qdisc_index, std::string &qdisc_type, std::string &params){
		size_t i = atoi(qdisc_index.c_str());
		if( i >= qdisc_vec.size() || i < 0 ) throw std::exception();
		qdisc_vec[i] = Create_qdisc(qdisc_type, params);
		std::cout << "\tqdisc ["<< i << "]:  "<< qdisc_vec[i]->describe() << "\n";
	}
	void push_back_qdisc(std::string &qdisc_type, std::string &params){
		qdisc_vec.push_back( Create_qdisc(qdisc_type, params) );
		std::cout << "\tqdisc ["<< qdisc_vec.size()-1 << "]:  "<< qdisc_vec[qdisc_vec.size()-1]->describe() << "\n";
	}
	void print_all(){
		std::cout << "\t" << filter.describe() << "\n";
		for(int i=0; i<qdisc_vec.size(); ++i){
			std::cout << "\tqdisc ["<< i << "]:  "<< qdisc_vec[i]->describe() << "\n";
		}
	}
	void set_qsize(size_t new_size){
		packets.set_queue_size(new_size);
	}
	nqueue():packets(10240){}
	bool packet_in( const npacket& p ){
		if( filter(p) ){
			packets.push(p);
			return true;
		}
		return false;
	}
	template<class Transport >
	bool	packet_out( Timer::base_type time, Transport& transport ){
		if(!qdisc_vec.size()) return false;
		for(int i=0; i<qdisc_vec.size(); ++i){
			if(!qdisc_vec[i]->can_pop( time, packets )) return false;
		}
		transport.send( packets.pop(time).data );
		return true;
	}
	void show_stat(Timer::base_type time){
		packets.show_stat(time);
	}
};
//----------------------------------------------------------------------------------
template<class Transport >
class queue_dispatcher : public concurrency::thread_object{
	Timer					timer;
	Transport&				transport;

	concurrency::cs_object	lock;
	concurrency::event		should_stop;
	concurrency::event		new_packet;
public:
	typedef std::map<std::string, nqueue> nq_list_t;
	nq_list_t	nq_list;

	void set_qsize(std::string &q_key, size_t new_size){
		concurrency::lock<concurrency::cs_object>	_locl(&lock);
		nq_list[q_key].set_qsize( new_size );
	}
	void clear_queue(std::string &q_key){
		concurrency::lock<concurrency::cs_object>	_locl(&lock);
		nq_list.erase(q_key);
	}
	void set_filter(std::string &q_key, std::string &filter){
		concurrency::lock<concurrency::cs_object>	_locl(&lock);
		nq_list[q_key].filter.set_options(filter); 
	}
	virtual unsigned long main(){
		Timer::base_type	start_time(timer.get_time());
		Timer::base_type	prev_time(start_time);
		while( !should_stop.isSignaled(0) ){
			new_packet.isSignaled(10);			
			Timer::base_type time = timer.get_time();

			for( bool	one_more_time=true; one_more_time; one_more_time = false ){
				concurrency::lock<concurrency::cs_object>	_locl(&lock);

				for(nq_list_t::iterator itr = nq_list.begin();itr != nq_list.end();++itr){
					one_more_time |= itr->second.packet_out( time, transport );
				}
			}			

			if (time - prev_time >= 500){
				prev_time = time - (time - prev_time - 500);
				concurrency::lock<concurrency::cs_object>	_locl(&lock);
				for(nq_list_t::iterator itr = nq_list.begin();itr != nq_list.end();++itr){
					std::cout << "STAT: QUEUE[" << itr->first << "] ";
					std::cout << " time="<< time - start_time;
					itr->second.show_stat(time);
				}
			}
		}
		return 0;
	}

	void print_all(){
		concurrency::lock<concurrency::cs_object>	_locl(&lock);
		for(nq_list_t::iterator itr = nq_list.begin();itr != nq_list.end();++itr){
			std::cout << "Queue["<< itr->first << "]:\n";
			itr->second.print_all();
			std::cout << "\n";			
		}
	}

	void set(std::string &q_key, std::string &qdisc_index, std::string &qdisc_name, std::string &params){
		concurrency::lock<concurrency::cs_object>	_locl(&lock);
		nq_list[q_key].set( qdisc_index, qdisc_name, params );
	}
	void push_back_qdisc(std::string &q_key, std::string &qdisc_name, std::string &params){
		concurrency::lock<concurrency::cs_object>	_locl(&lock);
		nq_list[q_key].push_back_qdisc(qdisc_name, params);
	}
	template<class P>
	void push(const P& p){
		concurrency::lock<concurrency::cs_object>	_locl(&lock);
		npacket packet( timer.get_time(), p );
		for(nq_list_t::iterator itr = nq_list.begin();itr != nq_list.end();++itr){
			if (itr->second.packet_in( packet )){
				new_packet.Set();
				return;
			}
		}
		transport.send( p );
	}
	queue_dispatcher(Transport& tr):transport(tr),new_packet(false,false){}
	
	void stop_join(){
		should_stop.Set();
		concurrency::thread_object::wait_for_exit();
	}
	~queue_dispatcher(){ stop_join(); }
};