#pragma once
#include "IThread.h"
class NetworkThread : public IThread
{
public:
	NetworkThread();
	virtual ~NetworkThread();

	virtual void Process();
};

