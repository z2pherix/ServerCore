#pragma once

#include <mutex>

#include "ObjectPool.h"
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

