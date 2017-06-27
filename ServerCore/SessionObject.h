#pragma once

#include <memory>

class Session;
class SessionObject
{
	std::shared_ptr<Session> sessionPtr_;

public:
	SessionObject();
	~SessionObject();
};

