#include <list>
#include "stdafx.h"

#include "ServerEngine.h"
#include "ServerApp.h"
#include "Parser.h"

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

class ServerTest : public ServerApp
{
	std::list<Session*>	clientList_;

public:

	ServerTest() {}
	~ServerTest() {}

	virtual void OnAccept( int port, void* session ) override
	{
		printf( "Accept Session [%d]\n", port );
		clientList_.push_back( static_cast<Session*>(session) );
	}

	virtual void OnClose( void* session ) override
	{
		printf( "Close Session\n" );
		clientList_.remove( static_cast<Session*>(session) );
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
		if( HEADER_SIZE > srcSize )
			return false;

		const PacketHeader* header = reinterpret_cast<const PacketHeader*>(src);
		if( static_cast<int>( header->packetSize_ + HEADER_SIZE ) > srcSize )
			return false;

		destSize = header->packetSize_ + HEADER_SIZE;
		memcpy( dest, src, header->packetSize_ + HEADER_SIZE );

		return true;
	}
};

int main()
{
	ServerEngine::GetInstance().InitializeEngine( new ServerTest );
	ServerEngine::GetInstance().InitializeParser( new ParserTest );

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

	ServerEngine::GetInstance().AddServerCommand( 1, [] ( Command& cmd ) -> unsigned int
	{
		char* query = "select * from city where Name like '%SE%';";
		ServerEngine::GetInstance().PushQuery( query, strlen(query)  );
		return 0;
	} );

	const char* connectStr = "DRIVER={MySQL ODBC 3.51 Driver};SERVER=127.0.0.1;USER=admin;PASSWORD=admin;Trusted_Connection=yes;Database=world";
	ServerEngine::GetInstance().InitializeDatabase( connectStr );
	
	ServerEngine::GetInstance().StartDatabase();
	ServerEngine::GetInstance().StartServer();

    return 0;
}

