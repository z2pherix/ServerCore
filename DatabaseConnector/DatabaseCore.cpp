#include <vector>
#include <array>
#include <json/json.h>

#include "../ServerCore/ObjectPool.h"
#include "../ServerCore/ObjectQueue.h"

#include "DatabaseCore.h"
#include "DatabaseThread.h"
#include "ODBCHandler.h"

#define DEFAULT_BUFFER_SIZE	2048
using BUFFER_TYPE = std::array<char, DEFAULT_BUFFER_SIZE>;

class DatabaseImplement
{
public:
	std::vector<std::shared_ptr<DatabaseThread>> databaseThreads_;

	ObjectQueue<BUFFER_TYPE>		queryQueue_;

	ObjectPool<Json::Value>			jsonPool_;
	ObjectPool<BUFFER_TYPE>			bufferPool_;
};

DatabaseCore::DatabaseCore()
{
}


DatabaseCore::~DatabaseCore()
{
	if( databaseImpl_ != nullptr )
		delete databaseImpl_;
}

std::unique_ptr<DatabaseCore> DatabaseCore::instance_;
std::once_flag DatabaseCore::onceFlag_;

DatabaseCore* DatabaseCore::GetInstance()
{
	std::call_once( onceFlag_, [] 
	{
		instance_.reset(new DatabaseCore);
	});

	return instance_.get();
}

void* DatabaseCore::AllocJson()
{
	return static_cast<void*>( databaseImpl_->jsonPool_.Alloc() );
}

void DatabaseCore::FreeJson( void* value )
{
	if( value == nullptr )
		return;

	databaseImpl_->jsonPool_.Free( static_cast<Json::Value*>( value ) );
}

char* DatabaseCore::AllocateBuffer()
{
	return reinterpret_cast<char*>( databaseImpl_->bufferPool_.Alloc() );
}

void DatabaseCore::FreeBuffer( char* buffer )
{
	databaseImpl_->bufferPool_.Free( reinterpret_cast<BUFFER_TYPE*>(buffer) );
}

bool DatabaseCore::InitDatabaseCore( const char* connectString )
{
	try
	{
		if( databaseImpl_ == nullptr )
		{
			databaseImpl_ = new DatabaseImplement();
		}

		unsigned int DATABASE_THREAD_COUNT = 4;
		for( unsigned int i = 0; i < DATABASE_THREAD_COUNT; ++i )
		{
			databaseImpl_->databaseThreads_.push_back( std::make_shared<DatabaseThread>() );
		}
	}
	catch( std::bad_alloc& )
	{
		return false;
	}

	for( auto thread : databaseImpl_->databaseThreads_ )
	{
		thread->Initialize( connectString );
	}
	
	return true;
}

void DatabaseCore::PushQuery( const char* query, size_t len )
{
	BUFFER_TYPE* buffer = reinterpret_cast<BUFFER_TYPE*>( AllocateBuffer() );

	if( buffer == nullptr )
		return;

	strncpy( buffer->data(), query, len );

	databaseImpl_->queryQueue_.Push( buffer );
}

bool DatabaseCore::PopQuery( char*& query )
{
	return databaseImpl_->queryQueue_.Pop( reinterpret_cast<BUFFER_TYPE*&>(query) );
}

void DatabaseCore::FreeQuery( char*& query )
{
	FreeBuffer( query );
}

void DatabaseCore::StartDatabase()
{
	for( auto thread : databaseImpl_->databaseThreads_ )
	{
		thread->StartThread();
	}
}