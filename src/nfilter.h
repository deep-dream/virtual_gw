#pragma once
#include "hl/smart_ptr.h"
#include "hl/params.h"
#include "npacket.h"
#include "print.h"
/*
//------------------------------------------------------------------------------
struct base_filter : public intrusive_counter{
	std::string desc;
	virtual bool check( const npacket& packet )=0;
	virtual std::string describe(){
		return desc;
	}
};
//------------------------------------------------------------------------------
struct ip_filter : public base_filter{
	int sip, smask;
	int dip, dmask;
	ip_filter():sip(0), smask(0), dip(0), dmask(0){	desc = "IP"; }
	ip_filter(int _sip, int _smask, int _dip, int _dmask):sip(_sip), smask(_smask), dip(_dip), dmask(_dmask){
		desc = "IP";
	}
	virtual bool check( const npacket& packet ){
		const ethernet_header& eh = reinterpret_cast<const ethernet_header&>(packet.data[0]);
		if( eh.ether_type != htons(0x0800)) return false;
		const ip_header& iph = reinterpret_cast<const ip_header&>(eh.data);
		int ip_len = (iph.ver_ihl & 0xf) * 4;
		if ((iph.saddr & smask) != sip) return false;
		if ((iph.daddr & dmask) != dip) return false;
		return true;
	}
};
//------------------------------------------------------------------------------
struct udp_filter : public ip_filter{
	int sport_lo, sport_hi;
	int dport_lo, dport_hi;
	udp_filter():sport_lo(0), sport_hi(255), dport_lo(0), dport_hi(255){ desc = "UDP";}
	void set_opt(std::string & opt){
		if (params( opt ).read("sport +([0-9]+)-([0-9]+)", port_lo, port_hi, false)){
			htons(sport_lo), htons(sport_hi)) );
		}
	}
	virtual bool check( const npacket& packet ){
		if (!ip_filter::check(packet)) return false;

		const ethernet_header& eh = reinterpret_cast<const ethernet_header&>(packet.data[0]);
		const ip_header& iph = reinterpret_cast<const ip_header&>(eh.data);
		int ip_len = (iph.ver_ihl & 0xf) * 4;
		const udp_header& udph = reinterpret_cast<const udp_header&>(*(eh.data + ip_len));
		if( iph.proto != IPPROTO_UDP ) return false;

		return true;
	}
};
//------------------------------------------------------------------------------
struct sport_filter : public udp_filter{
	sport_filter(int lo_sport, int hi_sport):sport_lo(lo_sport), sport_lo(lo_sport){ desc = "UDP sport";}
	virtual bool check( const npacket& packet ){
		if (!udp_filter::check(packet)) return false;

		const ethernet_header& eh = reinterpret_cast<const ethernet_header&>(packet.data[0]);
		const ip_header& iph = reinterpret_cast<const ip_header&>(eh.data);
		int ip_len = (iph.ver_ihl & 0xf) * 4;
		const udp_header& udph = reinterpret_cast<const udp_header&>(*(eh.data + ip_len));

		if ( udph.sport < sport_lo || sport_hi < udph.sport ) return false;

		return true;
	}
};
//------------------------------------------------------------------------------
struct dport_filter : public udp_filter{
	dport_filter(int lo_dport, int hi_dport):dport_lo(lo_dport), dport_lo(lo_dport){ desc = "UDP dport";}
	virtual bool check( const npacket& packet ){
		if (!udp_filter::check(packet)) return false;

		const ethernet_header& eh = reinterpret_cast<const ethernet_header&>(packet.data[0]);
		const ip_header& iph = reinterpret_cast<const ip_header&>(eh.data);
		int ip_len = (iph.ver_ihl & 0xf) * 4;
		const udp_header& udph = reinterpret_cast<const udp_header&>(*(eh.data + ip_len));

		if ( udph.dport < dport_lo || dport_hi < udph.dport ) return false;

		return true;
	}
};
*/
//------------------------------------------------------------------------------
class ip_filter{
	std::string id;
	int	ip;
	int	mask;
public:
	ip_filter():ip(0),mask(0){}
	ip_filter(const std::string& id, const std::string& opt){
		this->id = id;
		ip = 0;
		mask = 0;
		std::string _ip, _mask;
		if (params( opt ).read(id + " +([0-9]+.[0-9]+.[0-9]+.[0-9]+)", _ip, false)){
			mask = 0xFFFFFFFF;
			ip = ip_address( _ip.c_str() );
			if (params( opt ).read(id + " +[0-9]+.[0-9]+.[0-9]+.[0-9]+/([0-9]+)", _mask, false)){
				mask <<= 32 - (atoi(_mask.c_str()) && 0x1F);
				mask = ntohl( mask );
			}
		}
	}
	bool test(int _ip){ return (_ip & mask) == (ip & mask); }
	std::string describe(){
		return print("%s %s/", id.c_str(), ip_address(ip).c_str() ) + print("%s", ip_address(mask).c_str() );
	}
};
//------------------------------------------------------------------------------
class port_filter{
	std::string id;
	int	port_lo;
	int	port_hi;
public:
	port_filter():port_lo(0),port_hi(0xFFFF){}
	port_filter(const std::string& id, const std::string& opt){
		this->id = id;
		port_lo = 0;
		port_hi = 0xFFFF;
		if (params( opt ).read(id + " +([0-9]+)", port_lo, false)){
			port_hi = port_lo;
			params( opt ).read(id + " +[0-9]+-([0-9]+)", port_hi, false);
		}
		port_lo = htons(port_lo);
		port_hi = htons(port_hi);
	}
	bool test(unsigned short p){ return port_lo <= p && p <= port_hi; }
 	std::string describe(){
		return print("%s %u:%u", id.c_str(), port_lo, port_hi );
	}
};
//------------------------------------------------------------------------------
class nfilter{
	port_filter	sport;
	port_filter	dport;
	ip_filter	sip;
	ip_filter	dip;
public:
	void set_options(const std::string& opt){
		sport = port_filter("sport", opt);
		dport = port_filter("dport", opt);
		sip = ip_filter("sip", opt );
		dip = ip_filter("dip", opt );
		std::cout << describe() << "\n";
	}

	template<class P>
	bool operator()(const P& packet){
		//IP
		const ethernet_header& eh = reinterpret_cast<const ethernet_header&>(packet.data[0]);
		if( eh.ether_type != htons(0x0800)) return false;

		const ip_header& iph = reinterpret_cast<const ip_header&>(eh.data);
		int ip_len = (iph.ver_ihl & 0xf) * 4;

		if (!sip.test( iph.saddr.int_val ) ) return false;
		if (!dip.test( iph.daddr.int_val ) ) return false;
		
		if( iph.proto == IPPROTO_UDP ){
			const udp_header& udph = reinterpret_cast<const udp_header&>(*(eh.data + ip_len));
			if( !sport.test( udph.sport ) ) return false;
			if( !dport.test( udph.dport ) ) return false;
		}

		if( iph.proto == IPPROTO_TCP ){
			const tcphdr& tcph = reinterpret_cast<const tcphdr&>(*(eh.data + ip_len));
			if( !sport.test( tcph.th_sport ) ) return false;
			if( !dport.test( tcph.th_dport ) ) return false;
		}
		return true;
	}
	std::string describe(){
		return "Filter: " +	sip.describe() + " " + sport.describe() + " " + dip.describe() + " " + dport.describe();
	}
};
