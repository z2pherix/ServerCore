#pragma once
#include "NetworkModel.h"

#include <WinSock2.h>
#include <vector>

class Session;
class SelectModel :	public NetworkModel
{
	fd_set	fdReads_;
	int fdMax_ = 0;
	std::vector<Session*> fdList_;

public:
	SelectModel();
	virtual ~SelectModel();

	virtual bool InitNetworkModel();

	virtual bool AddSession( Session* newSession );
	virtual bool RemoveSession( Session* newSession );

	virtual void SelectSession();
};

