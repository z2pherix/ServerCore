#pragma once

#ifdef _WIN32
#include <windows.h>
#endif

#include <sql.h>
#include <sqlext.h>				// ODBC 3.X 라이브러리
#include <sqltypes.h>

class ODBCHandler
{
	SQLHSTMT	hStmt_ = nullptr;
	SQLHDBC		hDBC_ = nullptr;
	char		connectString_[MAX_PATH] = {0};

	bool _ReleaseHandle();
	bool _CheckSQLError( SQLRETURN ret, SQLSMALLINT nHandleType, SQLHANDLE hHandle, char* pErrMsg, char* _sqlState );

public:
	ODBCHandler();
	virtual ~ODBCHandler();

	bool Initialize();
	bool SetConnect( SQLHENV env, char* connectString );
	bool Reconnect( SQLHENV env );

	int  ExecuteQuery( const char* query );
};

