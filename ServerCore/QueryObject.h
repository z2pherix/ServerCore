#pragma once

#define MAX_QUERY_LEN	1024

struct QueryObject
{
	char queryMsg_[MAX_QUERY_LEN] = {0};
};
