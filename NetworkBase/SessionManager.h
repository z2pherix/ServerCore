#pragma once

#include <mutex>

#include "../ServerCore/ObjectPool.h"
#include "Session.h"


class SessionManager
{
	ObjectPool<Session>	sessionPool_;
	std::mutex	mutex_;

public:
	SessionManager();
	~SessionManager();

	Session* CreateSession( SOCKET socket );
	void RestoreSession( Session* session );
};

