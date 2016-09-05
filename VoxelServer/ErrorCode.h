#pragma once

enum class ERROR_CODE : short
{
	NONE = 0,

	INVALID_SESSION = 11,
	INVALID_REQ_PACKET =12,

	RECV_INCOMPLETE = 101
};