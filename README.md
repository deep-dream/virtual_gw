Sometimes for testing, debugging or development aims you need full control of the traffic between server
and client applications. Virtual GV uses the raw access to the Ethernet level packets to register new
virtual MAC address, request IP address from DHCP, and establish some basic routing/forwarding rules.

After that you can use it as a gateway between your applications. The functionality of Virtual GV allows to:
- Add delay
- Randomly drop packets
- Limit data transmission bandwidth

These rules maybe combined in a multiple chains and applied based on ip:port filter.

Virtual GV project is based on LIBPcap library. It's mostly tested on the windows platform. It's pretty simple and may be used for educational purpose.

Examples:

start VGW with 1 ethernet adapter (on local machine):

	vgw.exe -name=ServerName -i=1

stop VGW (from any machine at LAN):

	vgw.exe -name=ServerName -scmd stop

get VGW server IP by ServerName(from any machine at LAN):

	vgw.exe -name=ServerName -discovery


Know issues:

	-	Build procedure broken. Must be linked with wpcap.lib, packet.lib 


	-	On a 'smart' network card like Intel PRO 1000 the VGW doesn't work due to the 
		card configuration options about hardware IP, TCP acceleration

		If the options are turned off all work fine

ToDo:

	-	Use random or just unique MAC for each run (use the same MAC on the same PC)

		Idea:
		(http://en.wikipedia.org/wiki/MAC_address)
		Take the same MAC as the current PC but change the manufactory id.
		/code added to gate_way.cpp for MAC enumeration/

	+	done. Using last adapter MAC and changing second byte: mac.byte2 ^= 0x34;

	-	Create VC8 project for developing

	-	Add broadcast discovery support via UDP request. (Do it like arp_client.h)
		-	add name parameter to command line or use computer name instead to identify
			the server while receiving discovery replies

		

