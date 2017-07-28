#pragma once

#include <memory>
#include <thread>

class ODBCHandler;
class DatabaseThread
{
	volatile bool isRunning_ = false;
	
	std::shared_ptr<ODBCHandler> odbcHandler_;
	std::shared_ptr<std::thread> dbThread_;

public:
	DatabaseThread();
	virtual ~DatabaseThread();

	bool Initialize( const char* connectString );
	
	virtual void Process();
	virtual bool IsRunning() { return isRunning_; }

	virtual void StartThread();
	virtual void StopThread();
	virtual void JoinThread();
};

