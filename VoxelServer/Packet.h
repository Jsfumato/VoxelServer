enum class PACKETID : short
{
	NONE = -1,

	REQ_PING = 0,
	RES_PING = 1,

	REQ_LOGIN = 2,
	RES_LOGIN = 3,

	REQ_REGISTER_USER = 4,
	RES_REGISTER_USER = 5,

	REQ_GET_MAPDATA = 10,
	RES_GET_MAPDATA = 11,

	REQ_SAVE_MAP_NAME_INFO = 20,
	RES_SAVE_MAP_NAME_INFO = 21,
	REQ_SAVE_MAP_DATA = 22,
	RES_SAVE_MAP_DATA = 23,

	REQ_GET_DUNGEON_LIST = 30,
	RES_GET_DUNGEON_LIST = 31,
};


#pragma pack(push, 1)

constexpr int PACKET_HEADER_SIZE = 4;

struct PacketHeader
{
	PACKETID	pID;
	short		bodySize;
};

struct PacketBody {};

struct ResLoginPacketBody
{
	char uUserID[10] = { 0 };
};

struct ResSaveMapNameInfoBody
{
	char isComplete[1] = { 0 };
};

#pragma pack(pop)