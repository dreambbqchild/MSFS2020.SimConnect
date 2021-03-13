#include "BroadcastSocket.h"
#include "NetworkData.h"
#include <iphlpapi.h>
#include <iostream>
WSADATA wsaData = { 0 };
const int ONE = 1;

ULONG GetBroadcastAddress() 
{
    NetworkData<PMIB_IPADDRTABLE> ipAddrTable(GetIpAddrTable, FALSE);
    NetworkData<PMIB_IFTABLE> ifTable(GetIfTable, TRUE);	
    PMIB_IFROW rowUsing = nullptr;
    ULONG result = 0;

	for (DWORD i = 0; i < ipAddrTable.data->dwNumEntries; i++) 
    {
        auto currentRow = &ifTable.data->table[ipAddrTable.data->table[i].dwIndex - 1];
        if (currentRow->dwSpeed != UINT_MAX && (currentRow->dwType == IF_TYPE_ETHERNET_CSMACD || currentRow->dwType == IF_TYPE_IEEE80211))
        {
            if (!rowUsing || rowUsing->dwSpeed < currentRow->dwSpeed)
            {
                rowUsing = currentRow;
                result = ipAddrTable.data->table[i].dwAddr | ~ipAddrTable.data->table[i].dwMask;
            }
        }
	}	

	if (result == 0)
	{
		std::cout << "Could not find broadcast address :(" << std::endl;
		exit(1);
	}

	return result;
}

BroadcastSocket::BroadcastSocket() : s(INVALID_SOCKET)
{
	addr.sin_family = AF_INET;
	addr.sin_port = htons(49002);	
}

void BroadcastSocket::Open() 
{
	if (s == INVALID_SOCKET)
	{
		addr.sin_addr.S_un.S_addr = GetBroadcastAddress();
		s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		setsockopt(s, SOL_SOCKET, SO_BROADCAST, (const char*)&ONE, sizeof(ONE));
	}
}

void BroadcastSocket::Send(const char* data)
{
	if (s != INVALID_SOCKET)
		sendto(s, data, (int)strlen(data), 0, (sockaddr*)&addr, sizeof(addr));
}

void BroadcastSocket::Close()
{
	if (s != INVALID_SOCKET) 
	{
		closesocket(s);
		s = INVALID_SOCKET;
	}
}

BroadcastSocket::~BroadcastSocket() 
{
	Close();
}

void BroadcastSocket::Starting()
{
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NOERROR)
	{
		std::cout << "Could not start WinSock :(" << std::endl;
		exit(1);
	}	
}

void BroadcastSocket::Quitting()
{
	WSACleanup();
}