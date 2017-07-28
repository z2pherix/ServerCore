#pragma once

class ServerApp
{
public:
	ServerApp() {}
	virtual ~ServerApp() {}

	virtual void OnAccept( int /*port*/, void* /*session*/ ) {}
	virtual void OnClose( void* /*session*/ ) {}
};
