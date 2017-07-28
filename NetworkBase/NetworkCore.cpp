#include "NetworkCore.h"
#include "SessionManager.h"
#include "Accepter.h"
#include "Socket.h"

#include "../ServerCore/Packet.h"

#include "EpollModel.h"
#include "IOCPModel.h"
#include "SelectModel.h"

#ifdef _WIN32
#pragma comment(lib, "Ws2_32.lib")
#endif

class NetworkImplement
{
public:

	std::shared_ptr<SessionManager>			sessionManager_ = nullptr;
	std::shared_ptr<Accepter>				accepter_ = nullptr;
	std::shared_ptr<NetworkModel>			networkModel_ = nullptr;
};

NetworkCore::NetworkCore()
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
}


NetworkCore::~NetworkCore()
{
	WSACleanup();

	if( networkImpl_ != nullptr )
		delete networkImpl_;
}

std::unique_ptr<NetworkCore> NetworkCore::instance_;
std::once_flag NetworkCore::onceFlag_;

NetworkCore* NetworkCore::GetInstance()
{
	std::call_once( onceFlag_, [] 
	{
		instance_.reset(new NetworkCore);
	});

	return instance_.get();
}

bool NetworkCore::InitNetworkCore( funcOnAccept_t onAccpet, funcOnClose_t onClose )
{
	try
	{
		networkImpl_ = new NetworkImplement();

		networkImpl_->sessionManager_ = std::make_shared<SessionManager>();

#ifdef _WIN32
			networkImpl_->networkModel_ = std::make_shared<IOCPModel>();
#else
			networkImpl_->networkModel_ = std::make_shared<EpollModel>();
#endif

	}
	catch( std::bad_alloc& )
	{
		return false;
	}

	if( networkImpl_->networkModel_->InitNetworkModel() == false )
		return false;

	onAccept_ = onAccpet;
	onClose_ = onClose;

	return true;
}

bool NetworkCore::InitAccepter()
{
	try
	{
		networkImpl_->accepter_ = std::make_shared<Accepter>();

		if( networkImpl_->accepter_->InitAccepter() == false )
			return false;

		networkImpl_->accepter_->StartThread();
	}
	catch( std::bad_alloc& )
	{
		return false;
	}

	return true;
}

bool NetworkCore::AddAcceptPort( int port )
{
	if( networkImpl_->accepter_ == nullptr )
		return false;

	return networkImpl_->accepter_->AddAcceptPort( port );
}

Session* NetworkCore::CreateSession( Socket& socket )
{
	return networkImpl_->sessionManager_->CreateSession( socket.GetSocket() );
}

void NetworkCore::AddSession( Session* newSession, int acceptPort )
{
	networkImpl_->networkModel_->AddSession( newSession );

	onAccept_( acceptPort, newSession );
}

void NetworkCore::CloseSession( Session* session )
{
	if( session == nullptr )
		return;

	networkImpl_->networkModel_->RemoveSession( session );
	networkImpl_->sessionManager_->RestoreSession( session );

	session->CleanUp();

	onClose_( session );
}

void NetworkCore::SelectSession( std::vector<SessionEvent>& sessionList )
{
	networkImpl_->networkModel_->SelectSession( sessionList );
}