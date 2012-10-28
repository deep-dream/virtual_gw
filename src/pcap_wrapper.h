#pragma once
#include <string>
#include <vector>
#include <iostream>
#include "pcap/include/pcap.h"
#include "pcap/include/remote-ext.h"
#include "hl/regexp.h"

namespace winpcap{
	
	struct pcap_packet{
		struct pcap_pkthdr *header;
		unsigned char const *pkt_data;
		
		pcap_packet():header(0),pkt_data(0){}	
	};

	class pcap_device_list{
	public:
		struct	dev_info{
			std::string	name;
			std::string	description;
			std::string	ip_address;
			dev_info(const std::string& n,const std::string& d, const std::string& ip):name(n),description(d),ip_address(ip){}
		};
	private:
		typedef std::vector<dev_info>	dev_list_t;
		dev_list_t devices;
		char errbuf[PCAP_ERRBUF_SIZE];
		pcap_if_t *alldevs;
	public:
		pcap_device_list():alldevs(0){
			if(pcap_findalldevs(&alldevs, errbuf) == -1)
			{
				fprintf(stderr,"Error in pcap_findalldevs: %s\n", errbuf);
				throw std::exception();
			}
			pcap_if_t *d;
			for(d = alldevs; d; d = d->next)
				if( d->addresses ){
					pcap_addr_t *addr;
					for(addr = d->addresses; addr; addr = addr->next){
						if( addr->addr->sa_family == 2 ){
							char buffer[100];
							unsigned char* bytes = (unsigned char*)addr->addr->sa_data+2;
							sprintf_s(buffer, sizeof(buffer), "%i.%i.%i.%i",
								int(bytes[0]),int(bytes[1]),int(bytes[2]),int(bytes[3])
								);
							devices.push_back(dev_info(d->name,d->description, buffer ));
							break;
						}
					}
				}					
		}
		const dev_info &operator[] (size_t index) const {
			if( index >= devices.size() ){
				throw std::exception("\nAdapter number out of range.\n");
			}
			return devices[index]; 
		}
		size_t size(){
			return devices.size();
		}
		~pcap_device_list(){
			if (alldevs){
				pcap_freealldevs(alldevs);
			}
		}
	};
	
	class pcap_device{
		char errbuf[PCAP_ERRBUF_SIZE];
		pcap_t *adhandle;
		pcap_device(){}
		
		void init(const std::string& device_name){
			if((adhandle = pcap_open(device_name.c_str(),	// name of the device
				65536,		// 65536 grants that the whole packet will be captured on every link layer.
				PCAP_OPENFLAG_PROMISCUOUS |
				PCAP_OPENFLAG_NOCAPTURE_LOCAL |
				PCAP_OPENFLAG_MAX_RESPONSIVENESS,
				500,							// read timeout
				NULL,							// remote authentication
				errbuf							// error buffer
				)) == NULL)
			{
				fprintf(stderr,"\nUnable to open the adapter. %s is not supported by WinPcap\n", device_name.c_str());
				throw std::exception();
			}

			std::cout << pcap_lib_version() <<"\n";

			regexp rg("WinPcap version ([0-9]+)");
			if( rg.match(pcap_lib_version()) ){
				if( atoi(rg[1].c_str()) != 4 ){
					std::cout << "Warning! This program required version 4.0 of WinPCAP!\n";
				}
			}
			else{
				throw std::exception("ERROR: Can't get pcap version!\n");
			}

			if(pcap_datalink(adhandle) != DLT_EN10MB)
				throw std::exception("\nThis program works only on Ethernet networks.\n");
		}
	public:
		pcap_device(const pcap_device_list::dev_info& device_info){ init(device_info.name); }
		pcap_device(const std::string& device_name){ init(device_name); }
		
		~pcap_device(){ pcap_close(adhandle); }


		template<class T>
		void send(const std::vector<T>& data ){
			send((const unsigned char*)&(data[0]), data.size()*sizeof(T));
		}

		void send(const unsigned char *buffer, const int len){
			if (pcap_sendpacket(adhandle, buffer, len ) != 0){
				fprintf(stderr,"\nError sending the packet: \n", pcap_geterr(adhandle));
			}		
		}
		int get_packet(pcap_packet &packet){
			return pcap_next_ex( adhandle, &packet.header, &packet.pkt_data);
		};
	};

}
