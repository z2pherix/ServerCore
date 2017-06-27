#include "WorkThread.h"
#include "ServerEngine.h"
#include "Packet.h"

WorkThread::WorkThread()
{
}


WorkThread::~WorkThread()
{
}

void WorkThread::Process()
{
	while( IsRunning() == true )
	{
		Command command;
		if( ServerEngine::GetInstance().PopCommand( command ) == false )
			continue;

		if( command.cmdMessage_ == nullptr )
			continue;

		CommandFunction_t workFunc = ServerEngine::GetInstance().GetServerCommand( command.cmdID_ );

		if( workFunc != nullptr )
		{
			workFunc( command );
		}

		if( command.cmdType_ == COMMAND_NETWORK )
		{
			ServerEngine::GetInstance().FreePacket( static_cast<Packet*>(command.cmdMessage_) );
		}

	}
}