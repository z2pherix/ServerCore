#pragma once

#define DEFAULT_BUFFER_SIZE	2048	

struct BufferObject
{
	char buffer_[DEFAULT_BUFFER_SIZE] = {0};
};

