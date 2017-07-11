#pragma once

#define DEFAULT_BUFFER_SIZE	2048	

struct BufferObject
{
	char buffer_[DEFAULT_BUFFER_SIZE] = {0};

	BufferObject() { memset( buffer_, 0, sizeof(char) * DEFAULT_BUFFER_SIZE ); }
};

