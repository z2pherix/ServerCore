#include <map>
#include <vector>
#include <stdio.h>
#include <signal.h>

#include "ServerApp.h"
#include "ServerEngine.h"

#include "ObjectPool.h"
#include "Parser.h"
#include "CommandQueue.h"

#include "Packet.h"
#include "WorkThread.h"
#include "NetworkThread.h"

#ifdef USE_BOOST_ASIO
#include "../NetworkAsio/NetworkCore.h"
#else
#include "../NetworkBase/NetworkCore.h"
#endif

#include "../DatabaseConnector/DatabaseCore.h"

struct BufferObject
{
	enum
	{
		DEFAULT_BUFFER_SIZE = 2048,
	};

	char buffer_[DEFAULT_BUFFER_SIZE] = {0};

	BufferObject() { memset( buffer_, 0, sizeof(char) * DEFAULT_BUFFER_SIZE ); }
};

class ServerImplement
{
public:
	std::map< COMMAND_ID, CommandFunction_t > serverCommand_;

	NetworkCore*					networkCore_ = nullptr;
	DatabaseCore*					databaseCore_ = nullptr;
	
	std::shared_ptr<IParser>		parser_ = nullptr;
	std::shared_ptr<ServerApp>		serverApp_ = nullptr;

	std::shared_ptr<CommandQueue>	workQueue_ = nullptr;
	std::shared_ptr<CommandQueue>	dbQueue_ = nullptr;

	ObjectPool<Packet>				packetPool_;
	ObjectPool<BufferObject>		bufferPool_;


	std::vector<std::shared_ptr<NetworkThread>>		networkThreads_;
	std::vector<std::shared_ptr<WorkThread>>		workThreads_;
};

std::unique_ptr<ServerEngine> ServerEngine::instance_;
std::once_flag ServerEngine::onceFlag_;

ServerEngine& ServerEngine::GetInstance()
{
	std::call_once( onceFlag_, [] 
	{
		instance_.reset(new ServerEngine);
	});

	return *instance_.get();
}

ServerEngine::ServerEngine()
{
}

ServerEngine::~ServerEngine()
{
	if( serverImpl_ != nullptr )
		delete serverImpl_;
}

void ServerEngine::StartServer()
{
	for( auto thread : serverImpl_->networkThreads_ )
	{
		if( thread != nullptr )
			thread->JoinThread();
	}
}

void ServerEngine::StopServer()
{
	for( auto thread : serverImpl_->networkThreads_ )
	{
		if( thread != nullptr )
			thread->StopThread();
	}
}

bool ServerEngine::InitializeEngine( ServerApp* application )
{
	if( application == nullptr )
		return false;

	try
	{
		serverImpl_ = new ServerImplement();
		serverImpl_->serverApp_.reset( application );

		serverImpl_->workQueue_ = std::make_shared<CommandQueue>();
		serverImpl_->dbQueue_ = std::make_shared<CommandQueue>();

		for( unsigned int i = 0; i < std::thread::hardware_concurrency(); ++i )
		{
			serverImpl_->workThreads_.push_back( std::make_shared<WorkThread>() );
			serverImpl_->networkThreads_.push_back( std::make_shared<NetworkThread>() );
		}
	}
	catch( std::bad_alloc& )
	{
		return false;
	}

	serverImpl_->packetPool_.SetMaxPoolSize( 32 * 10 );
	if( serverImpl_->packetPool_.Init() == false )
		return false;

	serverImpl_->networkCore_ = NetworkCore::GetInstance();
	if( serverImpl_->networkCore_->InitNetworkCore( std::bind( &ServerApp::OnAccept, this->serverImpl_->serverApp_.get(), std::placeholders::_1, std::placeholders::_2 ),
													std::bind( &ServerApp::OnClose, this->serverImpl_->serverApp_.get(), std::placeholders::_1 ) ) == false )
	{
		return false;
	}

	for( auto thread : serverImpl_->networkThreads_ )
	{
		thread->StartThread();
	}

	for( auto thread : serverImpl_->workThreads_ )
	{
		thread->StartThread();
	}

	signal( SIGABRT, [] (int param) 
	{
		printf("abort\n"); 
	} );

	signal( SIGINT, [] (int param) 
	{
		printf("abort\n"); 
	} );

	return true;
}

bool ServerEngine::InitializeParser( IParser* parser )
{
	if( parser == nullptr )
		return false;

	try
	{
		serverImpl_->parser_.reset( parser );
	}
	catch( std::bad_alloc& )
	{
		return false;
	}

	return true;
}

bool ServerEngine::InitializeAccepter()
{
	return serverImpl_->networkCore_->InitAccepter();
}

bool ServerEngine::AddAcceptPort( int port )
{
	return serverImpl_->networkCore_->AddAcceptPort( port );
}

Session* ServerEngine::CreateSession( Socket& socket )
{
	return serverImpl_->networkCore_->CreateSession( socket );
}

void ServerEngine::AddSession( Session* newSession, int acceptPort )
{
	serverImpl_->networkCore_->AddSession( newSession, acceptPort );
}

ServerApp* ServerEngine::GetServerApp()
{
	return serverImpl_->serverApp_.get();
}

bool ServerEngine::EncodePacket( const char* src, int srcSize, char* dest, int& destSize )
{
	return serverImpl_->parser_->encodeMessage( src, srcSize, dest, destSize );
}

bool ServerEngine::DecodePacket( const char* src, int srcSize, char* dest, int& destSize )
{
	return serverImpl_->parser_->decodeMessage( src, srcSize, dest, destSize );
}

Packet* ServerEngine::AllocatePacket()
{
	return serverImpl_->packetPool_.Alloc();
}

void ServerEngine::FreePacket( Packet* obj )
{
	serverImpl_->packetPool_.Free( obj );
}

void ServerEngine::PushCommand( Command& cmd )
{
	serverImpl_->workQueue_->Push( cmd );
}

bool ServerEngine::PopCommand( Command& cmd )
{
	return serverImpl_->workQueue_->Pop( cmd );
}

bool ServerEngine::InitializeDatabase( const char* connectString )
{
	serverImpl_->databaseCore_ = DatabaseCore::GetInstance();

	return serverImpl_->databaseCore_->InitDatabaseCore( connectString );
}

void ServerEngine::PushQuery( const char* query, size_t len )
{
	if( serverImpl_->databaseCore_ == nullptr )
		return;

	serverImpl_->databaseCore_->PushQuery( query, len );
}

void ServerEngine::StartDatabase()
{
	serverImpl_->databaseCore_->StartDatabase();
}

void ServerEngine::AddServerCommand( COMMAND_ID protocol, CommandFunction_t command )
{
	if( serverImpl_->serverCommand_.find( protocol ) == serverImpl_->serverCommand_.end() )
	{
		serverImpl_->serverCommand_.insert( std::pair< PROTOCOL_TYPE, CommandFunction_t >( protocol, command ) );
	}
	else
	{
		serverImpl_->serverCommand_[protocol] = command;
	}
}

CommandFunction_t ServerEngine::GetServerCommand( COMMAND_ID protocol )
{
	if( serverImpl_->serverCommand_.find( protocol ) == serverImpl_->serverCommand_.end() )
	{
		return nullptr;
	}

	return serverImpl_->serverCommand_[protocol];
}