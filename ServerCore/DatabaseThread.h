#pragma once
#include "IThread.h"

class ODBCHandler;
class DatabaseThread : public IThread
{
	std::shared_ptr<ODBCHandler> odbcHandler_;

public:
	DatabaseThread();
	virtual ~DatabaseThread();
	
	bool Initialize( const char* connectString );
	
	virtual void SetThreadCount( unsigned int /*count*/ ) {}
	virtual void Process();
};

