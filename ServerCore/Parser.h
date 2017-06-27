#pragma once

#include <memory>

#include "Packet.h"

class IParser
{
public:
	IParser() {}
	virtual ~IParser() {}

	virtual bool encodeMessage( const char* src, int srcSize, char* dest, int& destSize ) = 0;
	virtual bool decodeMessage( const char* src, int srcSize, char* dest, int& destSize ) = 0;
};

class ParserDefault : public IParser
{
public:
	ParserDefault() {}
	virtual ~ParserDefault() {}

	virtual bool encodeMessage( const char* src, int srcSize, char* dest, int& destSize )
	{
		destSize = srcSize;
		memcpy( dest, src, destSize );

		return true;
	}
	virtual bool decodeMessage( const char* src, int srcSize, char* dest, int& destSize )
	{
		destSize = srcSize;
		memcpy( dest, src, srcSize );

		return true;
	}
};
