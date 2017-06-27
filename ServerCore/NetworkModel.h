#pragma once

class Session;
class NetworkModel
{
public:
	NetworkModel();
	virtual ~NetworkModel();

	virtual bool InitNetworkModel() = 0;

	virtual bool AddSession( Session* newSession ) = 0;
	virtual bool RemoveSession( Session* newSession ) = 0;

	virtual void SelectSession() = 0;
};

