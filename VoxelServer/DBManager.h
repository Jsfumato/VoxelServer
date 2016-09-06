#pragma once
#include <iostream>
#include <deque>

#include <OdbcLib.h>

enum class SQL_TYPE :short
{
	NONE = 0,

	REGISTER_USER = 1,
	GET_USER_INFO = 2,

	SAVE_DUNGEON_NAME_INFO = 10,
	SAVE_DUNGEON_BLOCK_DATA = 11,

	GET_DUNGEON_LIST = 20,
	GET_MAP_DATA = 21,
};

#pragma pack(push, 1)
struct SQLBody {};

struct SQLPacket
{
	void Clear()
	{
		sType = SQL_TYPE::NONE;
		delete sBody;
	}

	SQL_TYPE sType;
	SQLBody* sBody;
	void* pSession;
};

struct SQLtoNetPacket
{
	void*	pSession;	// Session*
	short	packetID;	// PACKETID
	char*		pBuffer;
	unsigned short length;
};

struct GetUserInfoBody : SQLBody
{
	char email[50] = { 0 };
	char password[20] = { 0 };
	short emailLen = 0;
	short passwordLen = 0;
};

struct RegisterUserInfoBody : SQLBody
{
	char email[50] = { 0 };
	char password[20] = { 0 };
	short emailLen = 0;
	short passwordLen = 0;
};

struct SaveDungeonNameInfoBody : SQLBody
{
	char uID[10] = { 0 };
	char dName[10] = { 0 };
	char dInfo[30] = { 0 };
	short roomNum;
};

struct MapData
{
	short posX;
	short posY;
	short posZ;
	short bType;
	float rotate;
};

struct SaveDungeonMapDataBody : SQLBody
{
	short roomNum;
	short blockNum;
	char uID[10] = { 0 };
	char dID[10] = { 0 };
	MapData* dArr;
};

struct GetDungeonListBody : SQLBody
{
	short numToGet;
};

struct GetMapDataPacket : SQLBody
{
	char uID[10] = { 0 };
	char dID[10] = { 0 };
};

#pragma pack(pop)


class DBManager
{
public:
	DBManager();
	~DBManager();

	bool InitMySQL();
	SQLtoNetPacket SQLProcess(); // return PACKETID
	void AddToSqlQueue(SQLPacket sPkt);

private:
	Odbc* m_MySQL;
	std::deque<SQLPacket> SQLqueue;
};

