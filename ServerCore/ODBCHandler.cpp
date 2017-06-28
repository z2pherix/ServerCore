#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
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

	SQLCHAR       sqlState[6] = {0};
	SQLCHAR		  sqlMsg[SQL_MAX_MESSAGE_LENGTH] = {0};
	SQLINTEGER    nError = 0;
	SQLSMALLINT   nRecordNum = 1;
	SQLSMALLINT   nMsgLen = SQL_MAX_MESSAGE_LENGTH;
	SQLRETURN     localRet = 0;	

	if( ( ret == SQL_ERROR ) || ( ret == SQL_SUCCESS_WITH_INFO ) )
	{
		while( ( localRet = SQLGetDiagRec( nHandleType, hHandle, nRecordNum, sqlState, &nError, sqlMsg,	SQL_MAX_MESSAGE_LENGTH,	&nMsgLen ) ) != SQL_NO_DATA )
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

bool ODBCHandler::SetConnect( SQLHENV hENV, char* connectString )
{
	// ODBC 연결을 위한 메모리 할당
	if( SQLAllocHandle( SQL_HANDLE_DBC, hENV, &hDBC_ ) != SQL_SUCCESS )
		return false;

	// ODBC를 이용한 데이터 베이스와의 연결 속성 설정
	SQLSetConnectAttr( hDBC_, SQL_ATTR_LOGIN_TIMEOUT, (SQLPOINTER)LOGIN_TIMEOUT, 0 );

	// 패킷 크기가 8KB일 때 최대의 성능을 낸다.
	SQLSetConnectAttr( hDBC_, SQL_ATTR_PACKET_SIZE, (SQLPOINTER)PACKETSIZE_OPT, 0 );

	SQLCHAR szOutConnect[MAX_PATH] = {0};
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

bool ODBCHandler::Reconnect( SQLHENV hENV )
{

	if( hDBC_ != nullptr )
	{
		_ReleaseHandle();

		SQLDisconnect( hDBC_ );
		SQLFreeHandle( SQL_HANDLE_DBC, hDBC_ );
		hDBC_ = nullptr;		
	}

	bool retval = this->SetConnect( hENV, connectString_ );

	if( !retval )
	{
		return false;
	}

	return true;
}

int ODBCHandler::ExecuteQuery( const char* query )
{
	if( hStmt_ == nullptr )
		return 0;

	char errorMsg[SQL_MAX_MESSAGE_LENGTH+128] = {0};
	char stateMsg[SQL_MAX_MESSAGE_LENGTH] = {0};

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
			SQLLEN displayLen = 0;
			SQLLEN ssType = 0;
			SQLNumResultCols( hStmt_ , &sNumResults );

			for (int col = 1; col <= sNumResults; col++)
			{
				SQLColAttribute( hStmt_, col, SQL_DESC_DISPLAY_SIZE, NULL, 0, NULL,	&displayLen );
				SQLColAttribute( hStmt_, col, SQL_DESC_CONCISE_TYPE, NULL, 0, NULL,	&ssType );

				// Allocate a buffer big enough to hold the text representation
				// of the data.  Add one character for the null terminator

				char* buffer = (char *)malloc((displayLen+1) * sizeof(char));

				SQLBindCol( hStmt_, col, SQL_C_TCHAR, (SQLPOINTER)buffer, (displayLen + 1) * sizeof(char), &pThisBinding->indPtr));


				// Now set the display size that we will use to display
				// the data.   Figure out the length of the column name

				TRYODBC(hStmt,
					SQL_HANDLE_STMT,
					SQLColAttribute(hStmt,
						iCol,
						SQL_DESC_NAME,
						NULL,
						0,
						&cchColumnNameLength,
						NULL));

				pThisBinding->cDisplaySize = max((SQLSMALLINT)cchDisplay, cchColumnNameLength);
				if (pThisBinding->cDisplaySize < NULL_SIZE)
					pThisBinding->cDisplaySize = NULL_SIZE;

				*pDisplay += pThisBinding->cDisplaySize + DISPLAY_FORMAT_EXTRA;

			}
			bool bNoData = false;

			if (sNumResults <= 0)
				break;

			do
			{
				RETCODE retCode = SQLFetch( hStmt_ );
				if( retCode == SQL_NO_DATA_FOUND )
				{
					bNoData = true;
				}
				else
				{
					SQLLEN cRowCount;

					TRYODBC(hStmt,
						SQL_HANDLE_STMT,
						SQLRowCount(hStmt,&cRowCount));

					if (cRowCount >= 0)
					{
						wprintf(L"%Id %s affected\n",
							cRowCount,
							cRowCount == 1 ? L"row" : L"rows");
					}
				}
			} while( bNoData == false );
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

