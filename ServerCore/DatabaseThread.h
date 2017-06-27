#pragma once
#include "IThread.h"
class DatabaseThread : public IThread
{
public:
	DatabaseThread();
	virtual ~DatabaseThread();

	virtual void Process();
};

