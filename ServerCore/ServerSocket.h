#pragma once
#include "Socket.h"

class ServerSocket : public Socket
{
	int listenPort_;

public:
	ServerSocket();
	virtual ~ServerSocket();

	int			GetPort() { return listenPort_; }

	SOCKET		InitServerSocket( int listenPort );
	int			Bind();
	int			Listen();
	SOCKET		Accept();
};

