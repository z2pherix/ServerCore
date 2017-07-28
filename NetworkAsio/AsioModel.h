#pragma once
#include "NetworkModel.h"

#include <boost/asio.hpp>

class AsioModel : public NetworkModel
{
	boost::asio::io_service		ioService_;

public:
	AsioModel();
	virtual ~AsioModel();

	virtual bool InitNetworkModel();

	virtual bool AddSession( Session* newSession );
	virtual bool RemoveSession( Session* newSession );

	virtual void SelectSession( std::vector<SessionEvent>& sessionList );
	virtual void StopNetworkModel();
};

