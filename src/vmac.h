#pragma once
#include "protocols/ethernet.h"

#include <Iphlpapi.h>
#pragma comment(lib,"iphlpapi.lib")

class virtual_mac{
	static void PrintMACaddress(unsigned char MACData[])
	{
		printf("MAC Address: %02X-%02X-%02X-%02X-%02X-%02X\n", 
			MACData[0], MACData[1], MACData[2], MACData[3], MACData[4], MACData[5]);
	}

	// Fetches the MAC address and prints it
	static void GetMACaddress(mac_address &vmac)
	{
		IP_ADAPTER_INFO AdapterInfo[16];       // Allocate information
		// for up to 16 NICs
		DWORD dwBufLen = sizeof(AdapterInfo);  // Save memory size of buffer

		DWORD dwStatus = GetAdaptersInfo(      // Call GetAdapterInfo
			AdapterInfo,                 // [out] buffer to receive data
			&dwBufLen);                  // [in] size of receive data buffer

		if (dwStatus != ERROR_SUCCESS ) return;

		PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo; // Contains pointer to
		// current adapter info
		do {
			PrintMACaddress(pAdapterInfo->Address); // Print MAC address
			memcpy(&vmac, pAdapterInfo->Address, 6);
			pAdapterInfo = pAdapterInfo->Next;    // Progress through
			// linked list
		}
		while(pAdapterInfo);                    // Terminate if last adapter
	}
	mac_address vmac;
public:
	const mac_address& mac(){ return vmac; }
	virtual_mac(){		
		printf("Local MAC's:\n");
		GetMACaddress(vmac);
		vmac.byte2 ^= 0x34;
		printf("Use virtual ");
		PrintMACaddress(&vmac.byte1);
	}
};
