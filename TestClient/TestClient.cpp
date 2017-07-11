#include <iostream>
#include <string>

#include "stdafx.h"

#include "Parser.h"
#include "Session.h"
#include "ServerEngine.h"
#include "ServerApp.h"

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
		if( header->packetSize_ + HEADER_SIZE > srcSize )
			return false;

		destSize = header->packetSize_ + HEADER_SIZE;
		memcpy( dest, src, destSize );

		return true;
	}
};

int main()
{
	ServerEngine::GetInstance();

	ServerEngine::GetInstance().InitializeEngine( MODEL_IOCP );
	ServerEngine::GetInstance().InitializeParser( new ParserTest );
	ServerEngine::GetInstance().InitializeApplication( new ServerApp );

	ServerEngine::GetInstance().AddServerCommand( 0, [] ( Command& cmd ) -> unsigned int
	{
		Packet* packet = static_cast<Packet*>(cmd.cmdMessage_);
		//printf("Recv : %s\n", packet->GetPacketData() );
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

	/*
	while( true )
	{
		std::string msg;
		std::cout << "Message : ";
		std::cin >> msg;

		Packet packet;

		packet.AddPacketData( msg.c_str(), static_cast<unsigned short>(msg.size()) );
		newSession->SendPacket( packet );
	}
	*/

	char brdMsg[1024] = {0};
	sprintf( brdMsg, "abcdedfghijklmnopqrstuvwxyz1234567890" );
	while( true )
	{
		Packet packet;
		
		packet.AddPacketData( brdMsg, strlen(brdMsg) );
		newSession->SendPacket( packet );
	}
    
	return 0;
}

