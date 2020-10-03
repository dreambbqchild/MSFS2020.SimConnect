#pragma once
#include <winsock2.h>
#include <Ws2tcpip.h>

class BroadcastSocket
{
private:
	SOCKET s;
	sockaddr_in addr;

public:
	BroadcastSocket();
	void Open();
	void Send(const char* data);
	void Close();
	virtual ~BroadcastSocket();
	
	static void Starting();
	static void Quitting();
};

