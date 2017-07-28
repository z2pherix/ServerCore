#include <json/json.h>

#include "DatabaseCore.h"
#include "DatabaseThread.h"
#include "ODBCHandler.h"

DatabaseThread::DatabaseThread()
{
}

DatabaseThread::~DatabaseThread()
{
}

bool DatabaseThread::Initialize( const char* connectString )
{
	try
	{
		odbcHandler_ = std::make_shared<ODBCHandler>();
	}
	catch( std::bad_alloc& )
	{
		return false;
	}

	if( odbcHandler_->Initialize() == false )
		return false;

	if( odbcHandler_->SetConnect( connectString ) == false )
		return false;

	return true;
}

void DatabaseThread::StartThread()
{
	isRunning_ = true;

	dbThread_ = std::make_shared<std::thread>( [&] () { Process(); } );
}

void DatabaseThread::StopThread()
{
	isRunning_ = false;
}

void DatabaseThread::JoinThread()
{
	if( dbThread_ != nullptr && dbThread_->joinable() == true )
	{
		dbThread_->join();
	}
}

void DatabaseThread::Process()
{
	while( IsRunning() == true )
	{
		char* query = nullptr;
		if( DatabaseCore::GetInstance()->PopQuery( query ) == false )
			continue;

		Json::Value* outValue = static_cast<Json::Value*>( DatabaseCore::GetInstance()->AllocJson() );
		
		odbcHandler_->ExecuteQuery( query, outValue );
		DatabaseCore::GetInstance()->FreeQuery( query );

		Json::StyledWriter writer;
		std::string outputConfig = writer.write( *outValue );
		printf("%s\n", outputConfig.c_str() );

		DatabaseCore::GetInstance()->FreeJson( outValue );
	}
}