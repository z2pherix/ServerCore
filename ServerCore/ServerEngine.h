#pragma once
#include <memory>
#include <mutex>
#include "Command.h"

class IParser;
class Session;
class Packet;
class Socket;

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
	void StopServer();

	bool InitializeEngine( ServerApp* application );
	bool InitializeParser( IParser* parser );

	bool InitializeAccepter();
	bool AddAcceptPort( int port );

	Session* CreateSession( Socket& socket );
	void AddSession( Session* newSession, int acceptPort );

	bool EncodePacket( const char* src, int srcSize, char* dest, int& destSize );
	bool DecodePacket( const char* src, int srcSize, char* dest, int& destSize );

	Packet* AllocatePacket();
	void FreePacket( Packet* obj );

	void PushCommand( Command& cmd );
	bool PopCommand( Command& cmd );

	bool InitializeDatabase( const char* connectString );
	void PushQuery( const char* query, size_t len );
	void StartDatabase();

	void AddServerCommand( COMMAND_ID protocol, CommandFunction_t command );
	CommandFunction_t GetServerCommand( COMMAND_ID protocol );
};

