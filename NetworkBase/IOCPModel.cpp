#include "../ServerCore/Packet.h"

#include "IOCPModel.h"
#include "Session.h"

IOCPModel::IOCPModel()
{
}


IOCPModel::~IOCPModel()
{
	DWORD dwBytesTransfer = 0;
	LPOVERLAPPED lpOverlapped = NULL;
	ULONG_PTR CompletionKey = NULL;

	PostQueuedCompletionStatus( iocpHandle_, dwBytesTransfer, CompletionKey, lpOverlapped );
}

bool IOCPModel::InitNetworkModel()
{
	iocpHandle_ = CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, 0, 0 );

	if( iocpHandle_ == NULL )
		return false;

	return true;
}

bool IOCPModel::AddSession( Session* newSession )
{
	HANDLE retHandle = CreateIoCompletionPort( (HANDLE)newSession->GetSocket() , iocpHandle_, (ULONG_PTR)newSession, 0 );
	if( retHandle != iocpHandle_ )
		return false;

	if( newSession->RecvPost() == false )
	{
		return false;
	}

	sessionList_.push_back( newSession );

	return true;
}

bool IOCPModel::RemoveSession( Session* session )
{
	sessionList_.remove( session );

	return true;
}

void IOCPModel::SelectSession( std::vector<SessionEvent>& sessionList )
{
	DWORD dwBytesTransfer = 0;
	LPOVERLAPPED lpOverlapped = NULL;
	ULONG_PTR CompletionKey = NULL;

	BOOL ret = GetQueuedCompletionStatus( iocpHandle_, &dwBytesTransfer, &CompletionKey, &lpOverlapped, INFINITE );

	if( lpOverlapped == nullptr )
	{
		return;
	}
	else
	{
		Session* session = ( Session* )CompletionKey;
		if( session )
		{
			if( ( ret == FALSE && GetLastError() == ERROR_NETNAME_DELETED )	||	// 강제 종료
				( ret == TRUE && dwBytesTransfer == 0 ) )						// Graceful 종료
			{
				sessionList.push_back( { SESSION_CLOSE, session, 0 } );
			}
			else
			{
				OVERLAPPEDEX* overlapped = ( OVERLAPPEDEX* )lpOverlapped;

				switch( overlapped->dwFlags )
				{
				case ASYNCFLAG_RECEIVE:
					{
						sessionList.push_back( { SESSION_RECV, session, static_cast<int>( dwBytesTransfer ) } );
					}
				break;
				}

				if( session->RecvPost() == false )
				{
					sessionList.push_back( { SESSION_CLOSE, session, 0 } );
				}
			}
		}
	}
}

void IOCPModel::StopNetworkModel()
{
}