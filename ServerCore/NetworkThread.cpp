#include <vector>

#include "NetworkThread.h"
#include "ServerEngine.h"

#ifdef USE_BOOST_ASIO
#include "../NetworkAsio/NetworkCore.h"
#include "../NetworkAsio/Session.h"
#else
#include "../NetworkBase/NetworkCore.h"
#include "../NetworkBase/Session.h"
#endif

void NetworkThread::Process()
{
	std::vector<SessionEvent> _sessionEvents;

	while( IsRunning() == true )
	{
		NetworkCore::GetInstance()->SelectSession( _sessionEvents );

		for( auto sessionEvent : _sessionEvents )
		{
			if( sessionEvent.session_ == nullptr )
				continue;

			switch( sessionEvent.event_ )
			{
			case SESSION_CLOSE:
				{
					NetworkCore::GetInstance()->CloseSession( sessionEvent.session_ );
				}
				break;

			case SESSION_RECV:
				{
					do
					{
						Packet* packet = ServerEngine::GetInstance().AllocatePacket();

						if( packet == nullptr )
							return;

						int packetSize = 0;
						if( ServerEngine::GetInstance().DecodePacket( sessionEvent.session_->RecvBufferPos(), static_cast<int>(sessionEvent.recvSize_), packet->GetPacketBuffer(), packetSize ) == false )
						{
							ServerEngine::GetInstance().FreePacket( packet );
							break;
						}
						else
						{
							ServerEngine::GetInstance().PushCommand( Command( static_cast<COMMAND_ID>(packet->GetProtocol()), static_cast<void*>(packet) ) );
							sessionEvent.session_->RecvBufferConsume( packet->GetPacketSize() );
							sessionEvent.recvSize_ -= packet->GetPacketSize();
						}

					} while( true );
				}
				break;
			}
		}

		_sessionEvents.clear();
	}
}