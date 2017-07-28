#include <stdio.h>
#include <string>

#include "Session.h"

Session::Session()
{
}

Session::Session( SOCKET socket )
{
	socket_.SetSocket( socket );
}


Session::~Session()
{
}

void Session::SetSocket( SOCKET socket )
{
	socket_.SetSocket( socket );
}

SOCKET Session::GetSocket()
{
	return socket_.GetSocket();
}


bool Session::RecvPost()
{
	if( socket_.GetSocket() == INVALID_SOCKET )
		return false;


	ZeroMemory( &overlappedRecv_, sizeof( OVERLAPPEDEX ) );
	overlappedRecv_.dwFlags = ASYNCFLAG_RECEIVE;

	// WSABUF 구조체 셋팅
	WSABUF wsaBuffer;
	wsaBuffer.buf = recvBuffer_.GetBufferPos();
	wsaBuffer.len = recvBuffer_.GetBufferSize();

	DWORD dwRecvBytes = 0;
	DWORD dwFlags = 0;	
	int nRet = WSARecv(	socket_.GetSocket(),
		&wsaBuffer,	1,
		&dwRecvBytes,
		&dwFlags,
		(OVERLAPPEDEX *)&overlappedRecv_,
		NULL);

	if( SOCKET_ERROR == nRet )
	{
		int nErrorCode = WSAGetLastError();
		if( nErrorCode != ERROR_IO_PENDING )
		{
			return false;
		}
	}

	return true;
}

void Session::RecvBufferConsume( int size )
{
	recvBuffer_.ConsumeBuffer( size );
}

int Session::RecvPacket()
{
	if( socket_.GetSocket() == INVALID_SOCKET )
	{
		return -1;
	}

	if( MAX_NET_BUFFER - recvBuffer_.GetBufferSize() <= 0 )
	{
		return -1;
	}

	int size = recv( socket_.GetSocket(), recvBuffer_.GetBufferPos(), MAX_NET_BUFFER - recvBuffer_.GetBufferSize(), 0  );

	if ( size < 0 )
	{
		int err = GetLastError();
		if( EWOULDBLOCK == err )
		{
			return 0;
		}
		else
		{
			return -1;
		}
	}
	else if ( size == 0 ) 
	{
		return -2;
	}

	return size;
}

void Session::CleanUp()
{
	socket_.CloseSocket();
}

int Session::SendPacket( Packet& packet )
{
	if( socket_.GetSocket() == INVALID_SOCKET )
		return 0;

	return send( socket_.GetSocket(), packet.GetPacketBuffer(), packet.GetPacketSize(), 0 );
}

void Session::Connect( const std::string& addr, std::uint32_t port )
{
	if( socket_.GetSocket() == INVALID_SOCKET )
	{
		socket_.CreateSocket();
	}

	socket_.Connect( addr.c_str(), port );
}

void Session::Disconnect( bool wait_for_removal )
{
	socket_.CloseSocket();
}

bool Session::IsConnected() const
{
	return socket_.IsConnected();
}

void Session::SetOnDisconnectionHandler( const std::function<void()>& /*disconnection_handler*/ )
{
}

int Session::SendBuffer(const std::vector<char>& data, std::size_t size_to_write)
{
	return send( socket_.GetSocket(), data.data(), static_cast<int>( size_to_write ), 0 );
}

int Session::RecvBuffer( std::vector<char>& data, std::size_t& recvSize )
{
	recvSize = recv( socket_.GetSocket(), recvBuffer_.GetBufferPos(), MAX_NET_BUFFER - recvBuffer_.GetBufferSize(), 0  );

	if ( recvSize < 0 )
	{
		int err = GetLastError();
		if( EWOULDBLOCK == err )
		{
			recvSize = 0;
		}
		else
		{
			return -1;
		}
	}

	return static_cast<int>( recvSize );
}