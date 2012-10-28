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

		

