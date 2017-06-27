#pragma once
#include "NetworkModel.h"

#include <WinSock2.h>

class IOCPModel : public NetworkModel
{
	HANDLE iocpHandle_;

public:
	IOCPModel();
	virtual ~IOCPModel();

	virtual bool InitNetworkModel();

	virtual bool AddSession( Session* newSession );
	virtual bool RemoveSession( Session* newSession );

	virtual void SelectSession();
};

