#pragma once
#include <memory>
#include <mutex>
#include "Command.h"

enum SERVER_MODEL
{
	MODEL_EPOLL,
	MODEL_IOCP,
	MODEL_SELECT,
};


class Socket;
class IParser;
class Session;
class Packet;

class ServerApp;
class ServerImplement;

class ServerEngine
{
private:

	static std::unique_ptr<ServerEngine> instance_;
	static std::once_flag				 onceFlag_;

	ServerImplement* serverImpl_ = nullptr;

	ServerEngine();
	ServerEngine(const ServerEngine& src) = delete;
	ServerEngine& operator=(const ServerEngine& rhs) = delete;

public:
	virtual ~ServerEngine();
	static ServerEngine& GetInstance();
	
	ServerApp* GetServerApp();

	void StartServer();
	
	bool InitializeEngine( SERVER_MODEL serverModel );
	bool InitializeParser( IParser* parser );
	bool InitializeApplication( ServerApp* application );

	bool InitializeAccepter();
	bool AddAcceptPort( int port );
	void StartAccepter();

	bool InitializeDatabase();
	bool AddDatabaseConnection();
	void StartDatabase();

	Session* CreateSession( Socket& socket );

	void AddSession( Session* newSession, int acceptPort );
	void CloseSession( Session* session );
	void SelectSession();
	
	bool EncodePacket( const char* src, int srcSize, char* dest, int& destSize );
	bool DecodePacket( const char* src, int srcSize, char* dest, int& destSize );

	Packet* AllocatePacket();
	void FreePacket( Packet* obj );

	void PushCommand( Command& cmd );
	bool PopCommand( Command& cmd );

	void AddServerCommand( COMMAND_ID protocol, CommandFunction_t command );
	CommandFunction_t GetServerCommand( COMMAND_ID protocol );
};

