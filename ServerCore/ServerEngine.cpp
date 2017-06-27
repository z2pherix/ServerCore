#include <map>
#include <stdio.h>

#include "Session.h"
#include "ServerApp.h"
#include "ServerEngine.h"

#include "Socket.h"
#include "NetworkModel.h"
#include "SessionManager.h"
#include "Parser.h"
#include "CommandQueue.h"
#include "Packet.h"

#include "Accepter.h"
#include "WorkThread.h"
#include "NetworkThread.h"
#include "DatabaseThread.h"

#include "IOCPModel.h"
#include "SelectModel.h"

class ServerImplement
{
public:
	std::map< COMMAND_ID, CommandFunction_t > serverCommand_;

	std::shared_ptr<SessionManager>			sessionManager_;
	std::shared_ptr<NetworkModel>			networkModel_;
	std::shared_ptr<IParser>				parser_;
	std::shared_ptr<ServerApp>				serverApp_;

	std::shared_ptr<Accepter>				accepter_;
	std::shared_ptr<WorkThread>				workThread_;
	std::shared_ptr<NetworkThread>			networkThread_;
	std::shared_ptr<DatabaseThread>			databaseThread_;

	std::shared_ptr<CommandQueue>			workQueue_;
	ObjectPool<Packet>						packetPool_;
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
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
}

ServerEngine::~ServerEngine()
{
	if( serverImpl_ != nullptr )
		delete serverImpl_;

	WSACleanup();
}

void ServerEngine::StartServer()
{
	serverImpl_->accepter_->JoinThread();
	serverImpl_->networkThread_->JoinThread();
	serverImpl_->workThread_->JoinThread();
}

bool ServerEngine::InitializeEngine( SERVER_MODEL serverModel )
{
	try
	{
		serverImpl_ = new ServerImplement();
		serverImpl_->workThread_ = std::make_shared<WorkThread>();
		serverImpl_->networkThread_ = std::make_shared<NetworkThread>();
		serverImpl_->sessionManager_ = std::make_shared<SessionManager>();
		serverImpl_->workQueue_ = std::make_shared<CommandQueue>();
		
		if( serverModel == MODEL_IOCP )
			serverImpl_->networkModel_ = std::make_shared<IOCPModel>();
		else
			serverImpl_->networkModel_ = std::make_shared<SelectModel>();
	}
	catch( std::bad_alloc& )
	{
		return false;
	}

	serverImpl_->packetPool_.SetMaxPoolSize( 32 * 10 );
	if( serverImpl_->packetPool_.Init() == false )
		return false;

	if( serverImpl_->networkModel_->InitNetworkModel() == false )
		return false;

	serverImpl_->workThread_->SetThreadCount(std::thread::hardware_concurrency());
	serverImpl_->networkThread_->SetThreadCount(std::thread::hardware_concurrency());

	serverImpl_->networkThread_->StartThread();
	serverImpl_->workThread_->StartThread();

	return true;
}

bool ServerEngine::InitializeApplication( ServerApp* application )
{
	if( application == nullptr )
		return false;

	try
	{
		serverImpl_->serverApp_.reset( application );
	}
	catch( std::bad_alloc& )
	{
		return false;
	}

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

ServerApp* ServerEngine::GetServerApp()
{
	return serverImpl_->serverApp_.get();
}

bool ServerEngine::InitializeAccepter()
{
	try
	{
		serverImpl_->accepter_ = std::make_shared<Accepter>();
	}
	catch( std::bad_alloc& )
	{
		return false;
	}

	if( serverImpl_->accepter_->InitAccepter() == false )
		return false;

	return true;
}

bool ServerEngine::AddAcceptPort( int port )
{
	return serverImpl_->accepter_->AddAcceptPort( port );
}

void ServerEngine::StartAccepter()
{
	serverImpl_->accepter_->StartThread();
}

bool ServerEngine::InitializeDatabase()
{
	try
	{
		serverImpl_->databaseThread_ = std::make_shared<DatabaseThread>();
	}
	catch( std::bad_alloc& )
	{
		return false;
	}

	return true;
}

bool ServerEngine::AddDatabaseConnection()
{
	return true;
}

void ServerEngine::StartDatabase()
{
	serverImpl_->databaseThread_->StartThread();
}

Session* ServerEngine::CreateSession( Socket& socket )
{
	return serverImpl_->sessionManager_->CreateSession( socket.GetSocket() );
}

void ServerEngine::AddSession( Session* newSession, int acceptPort )
{
	serverImpl_->networkModel_->AddSession( newSession );

	serverImpl_->serverApp_->OnAccept( acceptPort, newSession );
}

void ServerEngine::CloseSession( Session* session )
{
	serverImpl_->networkModel_->RemoveSession( session );
	serverImpl_->sessionManager_->RestoreSession( session );

	session->CleanUp();

	serverImpl_->serverApp_->OnClose( session );
}

void ServerEngine::SelectSession()
{
	serverImpl_->networkModel_->SelectSession();
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