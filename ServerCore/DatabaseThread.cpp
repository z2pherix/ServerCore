#include "DatabaseThread.h"
#include "ODBCHandler.h"
#include "ServerEngine.h"

DatabaseThread::DatabaseThread()
{
	odbcHandler_ = std::make_shared<ODBCHandler>();
}

DatabaseThread::~DatabaseThread()
{
}

bool DatabaseThread::Initialize( const char* connectString )
{
	if( odbcHandler_->Initialize() == false )
		return false;

	if( odbcHandler_->SetConnect( connectString ) == false )
		return false;

	return true;
}

void DatabaseThread::Process()
{
	while( IsRunning() == true )
	{
		Command command;
		if( ServerEngine::GetInstance().PopQuery( command ) == false )
			continue;

		if( command.cmdMessage_ == nullptr )
			continue;

		odbcHandler_->ExecuteQuery( (const char*)command.cmdMessage_ );

		ServerEngine::GetInstance().FreeQuery( command );
	}
}