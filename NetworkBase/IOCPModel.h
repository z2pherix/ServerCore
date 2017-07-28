#pragma once
#include "NetworkModel.h"

#include <WinSock2.h>
#include <list>

class Session;
class IOCPModel : public NetworkModel
{
	HANDLE					iocpHandle_;
	std::list<Session*>		sessionList_;

public:
	IOCPModel();
	virtual ~IOCPModel();

	virtual bool InitNetworkModel();

	virtual bool AddSession( Session* newSession );
	virtual bool RemoveSession( Session* newSession );

	virtual void SelectSession( std::vector<SessionEvent>& sessionList );
	virtual void StopNetworkModel();
};

