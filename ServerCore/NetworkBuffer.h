#pragma once

#define MAX_NET_BUFFER 24000

class NetworkBuffer
{
	char buffer_[MAX_NET_BUFFER] = {0};
	int bufferPos_ = 0;
	int bufferSize_ = MAX_NET_BUFFER;

public:
	NetworkBuffer()
	{
	}

	~NetworkBuffer()
	{
	}

	inline char* GetBufferOrg()
	{
		return buffer_;
	}

	inline char* GetBufferPos()
	{
		return buffer_ + bufferPos_;
	}

	inline int GetBufferSize()
	{
		return bufferSize_;
	}

	inline int ConsumeBuffer( int size )
	{
		memmove( buffer_, buffer_ + size, bufferSize_ - size );

		return bufferSize_;
	}
};