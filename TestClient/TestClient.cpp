#include <iostream>
#include <string>

#include "stdafx.h"

#include "Parser.h"
#include "ServerEngine.h"
#include "ServerApp.h"

#ifdef USE_BOOST_ASIO
#include "../NetworkAsio/Session.h"
#else
#include "../NetworkBase/Session.h"
#endif

#ifdef _WIN32
#pragma comment(lib, "NetworkBase.lib")
#pragma comment(lib, "ServerCore.lib")
#pragma comment(lib, "DatabaseConnector.lib")
#endif

#define SERVER_PORT 1500

class ParserTest : public IParser
{
public:
	ParserTest() {}
	virtual ~ParserTest() {}

	virtual bool encodeMessage( const char* src, int srcSize, char* dest, int& destSize )
	{
		destSize = srcSize;
		memcpy( dest, src, destSize );

		return true;
	}
	virtual bool decodeMessage( const char* src, int srcSize, char* dest, int& destSize )
	{
		if( HEADER_SIZE > srcSize )
			return false;

		const PacketHeader* header = reinterpret_cast<const PacketHeader*>(src);
		if( static_cast<int>(header->packetSize_ + HEADER_SIZE) > srcSize )
			return false;

		destSize = header->packetSize_ + HEADER_SIZE;
		memcpy( dest, src, destSize );

		return true;
	}
};

int main()
{
	ServerEngine::GetInstance();

	ServerEngine::GetInstance().InitializeEngine( new ServerApp );
	ServerEngine::GetInstance().InitializeParser( new ParserTest );

	ServerEngine::GetInstance().AddServerCommand( 0, [] ( Command& cmd ) -> unsigned int
	{
		Packet* packet = static_cast<Packet*>(cmd.cmdMessage_);
		printf("Recv : %s\n", packet->GetPacketData() );
		return 0;
	} );

	Socket socket;
	socket.CreateSocket();

	if( socket.Connect( "127.0.0.1", SERVER_PORT ) == false )
		return 0;

	Session* newSession = ServerEngine::GetInstance().CreateSession( socket );

	if( newSession == nullptr )
		return 0;

	ServerEngine::GetInstance().AddSession( newSession, 0 );

	//while( true )
	//{
	//	std::string msg;
	//	std::cout << "Message : ";
	//	std::cin >> msg;

	//	Packet packet;

	//	packet.AddPacketData( msg.c_str(), static_cast<unsigned short>(msg.size()) );
	//	newSession->SendPacket( packet );
	//}
	

	//while( true )
	{
		Packet packet;
		packet.SetProtocol( (PROTOCOL_TYPE)1 );
		newSession->SendPacket( packet );

		Sleep(1);
	}

	ServerEngine::GetInstance().StopServer();
    
	return 0;
}

