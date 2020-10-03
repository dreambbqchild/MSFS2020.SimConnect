#include "BroadcastSocket.h"
#include <iostream>
WSADATA wsaData = { 0 };
const int ONE = 1;

BroadcastSocket::BroadcastSocket() : s(INVALID_SOCKET)
{
	addr.sin_family = AF_INET;
	addr.sin_port = htons(49002);
	addr.sin_addr.S_un.S_addr = INADDR_BROADCAST;
}

void BroadcastSocket::Open() 
{
	if (s == INVALID_SOCKET)
	{
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