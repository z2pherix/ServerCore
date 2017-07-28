#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <vector>

class NetworkImplement;
class Session;
class Socket;

enum ENUM_SESSION_EVENT
{
	SESSION_CLOSE,
	SESSION_RECV,
	SESSION_SEND,
};

struct SessionEvent
{
	ENUM_SESSION_EVENT event_ = SESSION_CLOSE;
	Session* session_ = nullptr;
	int recvSize_ = 0;
};


class NetworkCore
{
	typedef std::function<void( int, void* )> funcOnAccept_t;
	typedef std::function<void( void* )>	funcOnClose_t;

	static std::unique_ptr<NetworkCore>	 instance_;
	static std::once_flag				 onceFlag_;

	NetworkImplement* networkImpl_ = nullptr;

	funcOnAccept_t onAccept_;
	funcOnClose_t onClose_;

	NetworkCore();
	NetworkCore(const NetworkCore& src) = delete;
	NetworkCore& operator=(const NetworkCore& rhs) = delete;

public:
	virtual ~NetworkCore();
	static NetworkCore* GetInstance();

	bool InitNetworkCore( funcOnAccept_t onAccpet, funcOnClose_t onClose );
	bool InitAccepter();
	bool AddAcceptPort( int port );

	Session* CreateSession( Socket& socket );

	void AddSession( Session* newSession, int acceptPort );
	void CloseSession( Session* session );

	void SelectSession( std::vector<SessionEvent>& sessionList );
};

