#include "pcap_wrapper.h"
#include "ctrl_break_handler.h"
#include "vmac.h"
#include "dhcp_client.h"
#include "arp_client.h"

#include "virtual_gw.h"
#include "client.h"

#pragma comment(lib,"Ws2_32.lib")

#include "hl/params.h"

int main(int argc, char** argv){
	try{
		params all_params( GetCommandLine() );
		if( all_params.check("[-/]h", false) ){
			std::cout << "Usage .exe [-i<number>] [-D]\n";
			std::cout << "-name=<Server name>  \n";
			std::cout << "-discovery	       Get status & ip of vgw\n";
			std::cout << "-D                   List available interfaces\n";
			std::cout << "-i=<n>               Interface number\n";
			std::cout << "-t=<n>               Number of tryes to find server with \"server name\"\n";
			std::cout << "-v                   Get WinPCAP version\n";
			std::cout << "-scmd command        Send command to server\n";
			std::cout << "	where command is:\n\
\n\
		stop server:\n\
			stop\n\
\n\
		add to queue disc delay (in ms)\n\
			q:queue_name push_back delay n\n\
\n\
		add to queue disc random loss\n\
			q:queue_name push_back rloss n\n\
\n\
		add to queue disc bandwidth (in bytes)\n\
		q:queue_name push_back band rate (n bytes per second) [burst (n bytes)] [max_delay (n ms)]\n\
\n\
		add to queue disc filter\n\
			q:queue_name filter [sip x.x.x.x[/mask]] [dip x.x.x.x[/mask]] [sport x[-y]] [dport x[-y]]\n\
\n\
		clear queue\n\
			q:queue_name reset\n\
\n\
		print all queues\n\
			print\n\
\n\
Examples:\n\
start VGW with 1 ethernet adapter (on local machine):\n\
	vgw.exe -name=ServerName -i=1\n\
\n\
stop VGW (from any machine at LAN):\n\
	vgw.exe -name=ServerName -scmd stop\n\
\n\
get VGW server IP by ServerName(from any machine at LAN):\n\
	vgw.exe -name=ServerName -discovery\n\
\n\n";

//			std::cout << "-get_statistics      \n";
			return 0;
		}
		
		if( all_params.check("-discovery", false) ){
			return vgw_client().discovery();
		}
		if( all_params.check("-scmd", false) ){
			return vgw_client().server_command();
		}

		std::string	server_name = "unknown";
		all_params.read("-name=([-A-Za-z0-9_]+)", server_name, false);

		int dev_num = 0;
		all_params.read("-i=([0-9]+)", dev_num, false);
		
		winpcap::pcap_device_list	dl;
		if (!dl.size()){
			throw std::exception("No interfaces found! Make sure WinPcap is installed.\n");
		}

		if( all_params.read("-D", dev_num, false) ){
			for(size_t i=0; i<dl.size(); ++i)
				std::cout << i << ": " << dl[i].description << "(ip:" << dl[i].ip_address << ")\n";
			return 0;
		}
		
		winpcap::pcap_device	pd( dl[dev_num] );
		if( all_params.check("-v", false) ){
			return 0;
		}

		std::cout << "VGW version 1.2.0\n";

		virtual_mac	fake_mac;
		
		dhcp_client<winpcap::pcap_device>		dhcp_cln(fake_mac.mac(), pd);
		arp_client<winpcap::pcap_device>		arp_cln(fake_mac.mac(), pd);
		router<winpcap::pcap_device>			router(arp_cln, fake_mac.mac(), pd);
		virtual_gw<winpcap::pcap_device>		vgw(server_name, router, dhcp_cln, pd);

		dhcp_cln.send_discover();

		std::cout << "\nListening on: " << dl[dev_num].description << "\n";
		Ctrl_break_handler ctrl_handler;

		winpcap::pcap_packet packet;			
		int res=0;
		while( (res = pd.get_packet(packet)) >= 0 && !Ctrl_break_handler::ctrl_break_pressed) {
			if (res){
				res = vgw.dispatch( packet.pkt_data, packet.header->len );
				if (res < 0 ) break;
				if (res) continue;
				if (!dhcp_cln._is_configured){
					if (dhcp_cln.dispatch( packet.pkt_data, packet.header->len )) continue;
					continue; // we have no ip
				}				
				arp_cln.set_ip(dhcp_cln._ip);
				if (arp_cln.dispatch( packet.pkt_data, packet.header->len )) continue;
				if (router.dispatch( packet.pkt_data, packet.header->len )) continue;
			}
		}	
		dhcp_cln.send_release();
		std::cout << "Graceful exit\n";
	}
	catch(const std::exception& ex){
		std::cout << "Exception: "<< ex.what() << "\n";		
	}
	catch(...){ std::cout << "Exception\n";}
	return 0;
}