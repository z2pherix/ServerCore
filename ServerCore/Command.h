#pragma once

using COMMAND_ID = unsigned int;

class Command
{
public:
	COMMAND_ID		cmdID_ = 0;
	void*			cmdMessage_ = nullptr;

	Command() {}
	Command( COMMAND_ID id, void* msg ) : cmdID_(id), cmdMessage_(msg) {}
};

using CommandFunction_t = unsigned int (*)( Command& command );


