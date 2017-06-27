#include <list>
#include "stdafx.h"

#include "ServerEngine.h"
#include "ServerApp.h"
#include "Parser.h"
#include "Session.h"

#define SERVER_PORT 1500

class ServerTest : public ServerApp
{
	std::list<Session*>	clientList_;

public:
	virtual void OnAccept( int port, Session* session )
	{
		printf( "Accept Session [%d]\n", port );
		clientList_.push_back( session );
	}
	virtual void OnClose( Session* session )
	{
		printf( "Close Session\n" );
		clientList_.remove( session );
	}

	std::list<Session*>& GetClientList() { return clientList_; }
};

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
		if( srcSize < HEADER_SIZE )
			return false;

		const PacketHeader* header = reinterpret_cast<const PacketHeader*>(src);
		if( header->packetSize_ + HEADER_SIZE < srcSize )
			return false;

		destSize = header->packetSize_ + HEADER_SIZE;
		memcpy( dest, src, header->packetSize_ + HEADER_SIZE );

		return true;
	}
};

int main()
{
	ServerEngine::GetInstance().InitializeEngine( MODEL_IOCP );
	ServerEngine::GetInstance().InitializeParser( new ParserTest );
	ServerEngine::GetInstance().InitializeApplication( new ServerTest );

	ServerEngine::GetInstance().InitializeAccepter();
	ServerEngine::GetInstance().AddAcceptPort( SERVER_PORT );

	ServerEngine::GetInstance().AddServerCommand( 0, [] ( Command& cmd ) -> unsigned int
	{
		Packet* packet = static_cast<Packet*>(cmd.cmdMessage_);

		ServerTest* serverApp = dynamic_cast<ServerTest*>(ServerEngine::GetInstance().GetServerApp());

		if( serverApp == nullptr )
			return __LINE__;

		Packet echoPacket;
		echoPacket.AddPacketData( packet->GetPacketData(), packet->GetPacketDataSize() );
		for( auto session : serverApp->GetClientList() )
		{
			session->SendPacket( echoPacket );
		}
		return 0;
	} );
	
	ServerEngine::GetInstance().StartAccepter();
	ServerEngine::GetInstance().StartServer();

    return 0;
}
