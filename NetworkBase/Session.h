#pragma once

#include <memory>
#include <functional>
#include <vector>

#include "../ServerCore/Packet.h"

#include "Socket.h"
#include "NetworkBuffer.h"

// 확장 OVERLAPPED 구조체
#define  ASYNCFLAG_SEND				0x01
#define  ASYNCFLAG_RECEIVE			0x02

typedef struct OVERLAPPEDEX : OVERLAPPED
{
	DWORD dwFlags;
} OVERLAPPEDEX;

class Session
{
	Socket socket_;
	
	OVERLAPPEDEX overlappedSend_;
	OVERLAPPEDEX overlappedRecv_;

	NetworkBuffer recvBuffer_;

public:
	Session();
	Session( SOCKET socket );
	virtual ~Session();

	void SetSocket( SOCKET socket );
	SOCKET GetSocket();

	bool RecvPost();
	int  RecvPacket();
	
	char* RecvBufferPos() { return recvBuffer_.GetBufferOrg(); }
	void  RecvBufferConsume( int size );

	int  SendPacket( Packet& packet );

	void CleanUp();

	void Connect( const std::string& addr, std::uint32_t port );
	void Disconnect( bool wait_for_removal = false );
	bool IsConnected() const;

	void SetOnDisconnectionHandler( const std::function<void()>& disconnection_handler );	
	int  SendBuffer( const std::vector<char>& data, std::size_t size_to_write );
	int  RecvBuffer( std::vector<char>& data, std::size_t& recvSize );
};
