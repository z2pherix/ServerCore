#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <vector>

#include "json/json.h"

#include "ServerEngine.h"
#include "ODBCHandler.h"

enum CONNECT_ATTRIBUTE
{
	LOGIN_TIMEOUT = 10,
	NOSCAN_OPT = 1,
	PACKETSIZE_OPT = 8192,
};

ODBCHandler::ODBCHandler()
{
}

ODBCHandler::~ODBCHandler( void )
{
	SQLDisconnect( hDBC_ );
	SQLFreeHandle( SQL_HANDLE_DBC, hDBC_ );

	hDBC_ = nullptr;
}

bool ODBCHandler::Initialize()
{
	if( SQLAllocHandle( SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv_ ) == SQL_ERROR )
	{
		fwprintf( stderr, L"Unable to allocate an environment handle\n" );
		return false;
	}

	// Register this as an application that expects 3.x behavior,
	// you must register something if you use AllocHandle
	if( SQLSetEnvAttr( hEnv_, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0 ) != SQL_SUCCESS )
		return false;

	return true;
}

bool ODBCHandler::_ReleaseHandle()
{
	if( hStmt_ == nullptr )
		return false;

	if( SQLFreeHandle( SQL_HANDLE_STMT, hStmt_ ) != SQL_SUCCESS )
		return false;

	hStmt_ = nullptr;

	return true;
}

bool ODBCHandler::_CheckSQLError( SQLRETURN ret, SQLSMALLINT nHandleType, SQLHANDLE hHandle, char* pErrMsg, char* _sqlState )
{
	if( ret == SQL_SUCCESS )
		return true;

	SQLCHAR       sqlState[6] = { 0 };
	SQLCHAR		  sqlMsg[SQL_MAX_MESSAGE_LENGTH] = { 0 };
	SQLINTEGER    nError = 0;
	SQLSMALLINT   nRecordNum = 1;
	SQLSMALLINT   nMsgLen = SQL_MAX_MESSAGE_LENGTH;
	SQLRETURN     localRet = 0;

	if( ( ret == SQL_ERROR ) || ( ret == SQL_SUCCESS_WITH_INFO ) )
	{
		while( ( localRet = SQLGetDiagRec( nHandleType, hHandle, nRecordNum, sqlState, &nError, sqlMsg, SQL_MAX_MESSAGE_LENGTH, &nMsgLen ) ) != SQL_NO_DATA )
		{
			if( localRet == SQL_INVALID_HANDLE )
			{
				//[ SQL : INVALID_HANDLE ] CheckSQLError()에서 사용된 Handle이 유효하지 않음"
				if( pErrMsg )
					strcpy( pErrMsg, "CheckSQLError() Invalid HANDLE Error [1]" );

				return false;
			}
			if( pErrMsg )
			{
				if( _sqlState )
				{
					strcpy( _sqlState, (char*)sqlState );
				}

				sprintf( pErrMsg, "SQLState [%s] %s", sqlState, sqlMsg );
			}

			++nRecordNum;
		}

		if( ret == SQL_SUCCESS_WITH_INFO )
			return true;
		else
			return false;
	}
	else if( ret == SQL_INVALID_HANDLE )
	{
		//[ SQL : INVALID_HANDLE ] CheckSQLError()에서 사용된 Handle이 유효하지 않음
		if( pErrMsg )
		{
			sprintf( pErrMsg, "CheckSQLError() Invalid HANDLE Error [2]" );
		}

		return false;
	}

	return true;
}

bool ODBCHandler::SetConnect( const char* connectString )
{
	if( hEnv_ == nullptr )
		return false;

	// ODBC 연결을 위한 메모리 할당
	if( SQLAllocHandle( SQL_HANDLE_DBC, hEnv_, &hDBC_ ) != SQL_SUCCESS )
		return false;

	// ODBC를 이용한 데이터 베이스와의 연결 속성 설정
	SQLSetConnectAttr( hDBC_, SQL_ATTR_LOGIN_TIMEOUT, (SQLPOINTER)LOGIN_TIMEOUT, 0 );

	// 패킷 크기가 8KB일 때 최대의 성능을 낸다.
	SQLSetConnectAttr( hDBC_, SQL_ATTR_PACKET_SIZE, (SQLPOINTER)PACKETSIZE_OPT, 0 );

	SQLCHAR szOutConnect[MAX_PATH] = { 0 };
	SQLSMALLINT cbOutConnect = 0;

	// ODBC를 이용한 데이터 베이스와의 연결
	SQLRETURN retValue = SQLDriverConnect( hDBC_, NULL, (SQLCHAR*)connectString, SQL_NTS, szOutConnect, sizeof( szOutConnect ), &cbOutConnect, SQL_DRIVER_NOPROMPT );
	if( retValue == SQL_SUCCESS || retValue == SQL_SUCCESS_WITH_INFO )
	{
		SQLRETURN retValue = SQLAllocHandle( SQL_HANDLE_STMT, hDBC_, &hStmt_ );
		if( retValue == SQL_SUCCESS || retValue == SQL_SUCCESS_WITH_INFO )
		{
			// 쿼리의 실행 타임 아웃을 설정한다.
			//SQLSetStmtAttr( hStmt_, SQL_ATTR_QUERY_TIMEOUT, (SQLPOINTER)QUERY_TIMEOUT, 0 );
			//SQLSetStmtAttr( hStmt_, SQL_ATTR_ASYNC_ENABLE, (SQLPOINTER)SQL_ASYNC_ENABLE_ON, 0);
		}
		else
		{
			return false;
		}

		strcpy( connectString_, connectString );
		return true;
	}
	else
	{
		return false;
	}
}

bool ODBCHandler::Reconnect()
{
	if( hDBC_ != nullptr )
	{
		_ReleaseHandle();

		SQLDisconnect( hDBC_ );
		SQLFreeHandle( SQL_HANDLE_DBC, hDBC_ );
		hDBC_ = nullptr;
	}

	bool retval = this->SetConnect( connectString_ );

	if( !retval )
	{
		return false;
	}

	return true;
}

int ODBCHandler::ExecuteQuery( IN const char* query, OUT Json::Value& outValue )
{
	if( hStmt_ == nullptr )
		return 0;

	char errorMsg[SQL_MAX_MESSAGE_LENGTH + 128] = { 0 };
	char stateMsg[SQL_MAX_MESSAGE_LENGTH] = { 0 };

	SQLRETURN retValue = SQLExecDirect( hStmt_, (SQLCHAR*)query, SQL_NTS );
	switch( retValue )
	{
		case SQL_SUCCESS_WITH_INFO:
		{
			_CheckSQLError( retValue, SQL_HANDLE_STMT, hStmt_, errorMsg, stateMsg );
			// fall through
		}
		case SQL_SUCCESS:
		{
			SQLSMALLINT sNumResults = 0;
			SQLNumResultCols( hStmt_, &sNumResults );

			std::vector<char*> buffer;
			std::vector<char*> title;

			for( int col = 0; col < sNumResults; col++ )
			{
				buffer.push_back( ServerEngine::GetInstance().AllocateBuffer() );
				title.push_back( ServerEngine::GetInstance().AllocateBuffer() );

				SQLColAttribute( hStmt_, (col + 1), SQL_DESC_NAME, title[col], sizeof( title[col] ), NULL, NULL  );
				Json::Value param;
				outValue[title[col]] = param;
			}

			bool bNoData = false;

			do
			{
				for( int col = 0; col < sNumResults; col++ )
				{
					SQLLEN indPtr = 0;
					SQLBindCol( hStmt_, (col + 1), SQL_C_TCHAR, (SQLPOINTER)buffer[col], sizeof( buffer[col] ), &indPtr );
				}

				if( sNumResults <= 0 )
					break;

				RETCODE retCode = SQLFetch( hStmt_ );
				if( retCode == SQL_NO_DATA_FOUND )
				{
					bNoData = true;
				}

				for( int i = 0; i < sNumResults; ++i )
				{
					if( bNoData == false )
					{
						outValue[title[i]].append( buffer[i] );
					}
				}
			
			} while( bNoData == false );

			for( int i = 0; i < sNumResults; ++i )
			{
				ServerEngine::GetInstance().FreeBuffer( title[i] );
				ServerEngine::GetInstance().FreeBuffer( buffer[i] );
			}
		}
		break;

		case SQL_ERROR:
		{
			_CheckSQLError( retValue, SQL_HANDLE_STMT, hStmt_, errorMsg, stateMsg );
		}
		break;

		default:
			fwprintf( stderr, L"Unexpected return code %hd!\n", retValue );

	}

	return retValue;
}

