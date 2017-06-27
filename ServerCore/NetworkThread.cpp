#include "NetworkThread.h"
#include "ServerEngine.h"
#include "Session.h"

NetworkThread::NetworkThread()
{
}


NetworkThread::~NetworkThread()
{
}

void NetworkThread::Process()
{
	while( IsRunning() == true )
	{
		ServerEngine::GetInstance().SelectSession();
	}
}