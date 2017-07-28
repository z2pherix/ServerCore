#pragma once
#include <vector>

#include "NetworkCore.h"

class Session;
class NetworkModel
{
public:
	NetworkModel();
	virtual ~NetworkModel();

	virtual bool InitNetworkModel() = 0;

	virtual bool AddSession( Session* newSession ) = 0;
	virtual bool RemoveSession( Session* newSession ) = 0;

	virtual void SelectSession( std::vector<SessionEvent>& sessionList ) = 0;
	virtual void StopNetworkModel() = 0;
};

