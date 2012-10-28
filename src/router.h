#pragma once
#include "protocols/ethernet.h"
#include "protocols/ip.h"
#include "protocols/udp.h"
#include "protocols/tcp.h"
#include "dhcp_client.h"
#include "arp_client.h"
#include "nqueue.h"

#include <iostream>

template<class  Transport>
class router{
	Transport&				transport;
	arp_client<Transport>&	arp_cln;
	mac_address				fake_mac;
public:
	queue_dispatcher< Transport >	qm;


	router(arp_client<Transport>& a, const mac_address& mac, Transport& tr ):
	  arp_cln(a),fake_mac(mac),transport(tr),qm(tr){
		qm.start();
	  }

	bool dispatch(const unsigned char* data, int size){
		const ethernet_header& eh = reinterpret_cast<const ethernet_header&>(*data);
		
		if( eh.ether_type == htons(0x0800) && eh.dest == fake_mac )
		{
			static char buffer[0xFFFF];
			memcpy(buffer,data,size);
			ethernet_header& eh_new = reinterpret_cast<ethernet_header&>(buffer);
			ip_header& iph = reinterpret_cast<ip_header&>(eh_new.data);

			eh_new.source = fake_mac;
			// get MAC for IP
			if (arp_cln.get_mac(iph.daddr, eh_new.dest))
			{
				char s_port[255]={0};
				char d_port[255]={0};
				char flags[255]={0};

				// get port string
				if (IPPROTO_TCP == iph.proto)
				{
					char *ip_data = (char*)&iph.daddr + sizeof(ip_address);
					tcphdr* tcph = reinterpret_cast<tcphdr*>(ip_data);
					_itoa_s(htons(tcph->th_sport), s_port, sizeof(s_port), 10);
					_itoa_s(htons(tcph->th_dport), d_port, sizeof(s_port), 10);
					
					sprintf(flags,"%s %s %s",
						(tcph->syn?"SYN":""),
						(tcph->ack?"ACK":""),
						(tcph->psh?"PUSH":"")
						);
				}

//				printf("%s,%.2d ", timestr, header->ts.tv_usec);
//				std::cout << iph.saddr.c_str() <<":"<< s_port;
//				std::cout << " -> " << iph.daddr.c_str() << ":" << d_port << " len:" << size << " " << flags << "\n"; 

				qm.push( std::vector<char>(buffer, buffer + size) );
			}
			return true;
		}
		return false;
	}	
};