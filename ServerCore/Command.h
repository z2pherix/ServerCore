#pragma once

enum COMMAND_TYPE
{
	COMMAND_NETWORK,
	COMMAND_DBMSG,
};

using COMMAND_ID = unsigned int;

class Command
{
public:
	COMMAND_TYPE	cmdType_ = COMMAND_NETWORK;
	COMMAND_ID		cmdID_ = 0;
	void*			cmdMessage_ = nullptr;

	Command() {}
	Command( COMMAND_TYPE type, COMMAND_ID id, void* msg ) : cmdType_(type), cmdID_(id), cmdMessage_(msg) {}
};

using CommandFunction_t = unsigned int (*)( Command& command );


