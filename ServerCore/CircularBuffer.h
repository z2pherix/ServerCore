#pragma once

#define MAX_NET_BUFFER 24000

class CircularBuffer
{
	char buffer_[MAX_NET_BUFFER];
	int bufferPos_;
	int bufferSize_;

public:
	CircularBuffer()
	{
		bufferPos_ = 0;
		bufferSize_ = MAX_NET_BUFFER;
	}

	~CircularBuffer()
	{
	}

	inline char* GetBuffer()
	{
		return buffer_ + bufferPos_;
	}

	inline int GetBufferSize()
	{
		return bufferSize_;
	}

	inline int ConsumeBuffer( int size )
	{
		if( bufferPos_ + size > MAX_NET_BUFFER )
		{
			bufferPos_ = 0;
			bufferSize_ = MAX_NET_BUFFER;
		}
		else
		{
			bufferPos_ += size;
			bufferSize_ -= size;
		}
		
		return bufferSize_;
	}
};