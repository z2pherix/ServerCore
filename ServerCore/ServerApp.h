#pragma once

class Sessoin;
class ServerApp
{
public:
	ServerApp() {}
	virtual ~ServerApp() {}

	virtual void OnAccept( int /*port*/, Session* /*session*/ ) {};
	virtual void OnClose( Session* /*session*/ ) {};
};
