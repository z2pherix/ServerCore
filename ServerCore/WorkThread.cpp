#include "WorkThread.h"
#include "ServerEngine.h"
#include "Packet.h"

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

		ServerEngine::GetInstance().FreePacket( static_cast<Packet*>(command.cmdMessage_) );
	}
}