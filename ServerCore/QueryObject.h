#pragma once

#define MAX_QUERY_LEN	1024

struct QueryObject
{
	char queryMsg_[MAX_QUERY_LEN] = {0};

	QueryObject() { memset( queryMsg_, 0, sizeof(char) * MAX_QUERY_LEN ); }
};
