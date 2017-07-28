#include "AsioModel.h"

AsioModel::AsioModel()
{
}


AsioModel::~AsioModel()
{
}

bool AsioModel::InitNetworkModel()
{
	try
	{
		ioService_.run();
	}
	catch( std::exception & ex )
	{
		printf("%s\n", ex.what() );
	}

	return true;
}

bool AsioModel::AddSession( Session* newSession )
{
	return true;
}

bool AsioModel::RemoveSession( Session* newSession )
{
	return true;
}

void AsioModel::SelectSession( std::vector<SessionEvent>& sessionList )
{
}

void AsioModel::StopNetworkModel()
{
}