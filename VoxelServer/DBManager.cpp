#include "DBManager.h"
#include <math.h>
#include "Packet.h"



DBManager::DBManager()
{
	m_MySQL = Odbc::GetInstance();
}

DBManager::~DBManager()
{
}

bool DBManager::InitMySQL()
{
	m_MySQL = Odbc::GetInstance();
	bool ret = m_MySQL->Connect(L"Voxel", L"voxel", L"!!@@##$$");
	std::cout << "sql init : " << ret << std::endl;

	if (ret == false)
	{
		std::cout << "[ INFO ] MySQL connect fail..." << std::endl;
		return false;
	}

	std::cout << "[ INFO ] MySQL connected..." << std::endl;
	return true;
}

// return PACKETID
SQLtoNetPacket DBManager::SQLProcess()
{
	SQLtoNetPacket snPkt;
	snPkt.pSession = nullptr;
	snPkt.packetID = (short)PACKETID::NONE;

	if (SQLqueue.empty() == true)
		return snPkt;

	SQLPacket sPkt = SQLqueue.front();
	
	switch (sPkt.sType)
	{
	case SQL_TYPE::GET_USER_INFO :
	{
		GetUserInfoBody* body = (GetUserInfoBody*)sPkt.sBody;

		auto ret = m_MySQL->CallGetUserData(body->email, body->password);
		
		snPkt.packetID = (short)PACKETID::RES_LOGIN;
		snPkt.pSession = sPkt.pSession;
		snPkt.length = PACKET_HEADER_SIZE + 11;

		PacketHeader pHeader;
		pHeader.pID = PACKETID::RES_LOGIN;
		pHeader.bodySize = 11;

		int totalLen = PACKET_HEADER_SIZE + 11;
		char* tmp = new char[totalLen];

		memcpy(tmp, &pHeader, 4);

		if (ret.size() == 0)
		{
			bool result = false;
			memcpy(&tmp[4], &result, 1);
		}
		else
		{
			bool result = true;
			memcpy(&tmp[4], &result, 1);
			std::string uID = ret[0][2];
			std::copy(&uID.c_str()[0], &uID.c_str()[10], &tmp[5]);
		}

		snPkt.pBuffer = tmp;
	}
	break;

	case SQL_TYPE::REGISTER_USER:
	{
		GetUserInfoBody* body = (GetUserInfoBody*)sPkt.sBody;

		auto ret = m_MySQL->CallRegisterUser(body->email, body->password);

		snPkt.packetID = (short)PACKETID::RES_REGISTER_USER;
		snPkt.pSession = sPkt.pSession;
		snPkt.length = PACKET_HEADER_SIZE + 11;

		PacketHeader pHeader;
		pHeader.pID = PACKETID::RES_REGISTER_USER;
		pHeader.bodySize = 11;

		int totalLen = PACKET_HEADER_SIZE + 11;
		char* tmp = new char[totalLen];

		memcpy(tmp, &pHeader, 4);

		if (ret.size() == 0)
		{
			bool result = false;
			memcpy(&tmp[4], &result, 1);
		}
		else
		{
			bool result = true;
			memcpy(&tmp[4], &result, 1);
			std::string uID = ret[0][0];
			std::copy(&uID.c_str()[0], &uID.c_str()[10], &tmp[5]);
		}

		snPkt.pBuffer = tmp;
	}
	break;

	case SQL_TYPE::SAVE_DUNGEON_NAME_INFO:
	{
		SaveDungeonNameInfoBody* body = (SaveDungeonNameInfoBody*)sPkt.sBody;

		auto result = m_MySQL->CallSaveDungeonNameInfo(
			std::string(body->uID).substr(0, 10),
			std::string(body->dName).substr(0, 10),
			std::string(body->dInfo).substr(0, 30),
			body->roomNum
		);

		auto dID = result[0][0].c_str();

		snPkt.pSession = sPkt.pSession;
		snPkt.packetID = (short)PACKETID::RES_SAVE_MAP_NAME_INFO;
		snPkt.length = PACKET_HEADER_SIZE + 10;
		char* ret = new char[snPkt.length];

		PacketHeader pHeader;
		pHeader.pID = PACKETID::RES_SAVE_MAP_NAME_INFO;
		pHeader.bodySize = 10;

		memcpy(&ret[0], &pHeader, 4);
		std::copy(&dID[0], &dID[10], &ret[PACKET_HEADER_SIZE]);

		std::cout << "[ SQL Process ] SAVE_DUNGEON_NAME_INFO | dID : " << dID << std::endl;

		snPkt.pBuffer = ret;
	}
	break;

	case SQL_TYPE::SAVE_DUNGEON_BLOCK_DATA:
	{
		SaveDungeonMapDataBody* body = (SaveDungeonMapDataBody*)sPkt.sBody;

		std::string uID = body->uID;
		std::string dID = body->dID;
		short roomNum = body->roomNum;
		short blockNum = body->blockNum;

		bool result = false;
		for (int i = 0; i < body->blockNum; ++i)
		{
			MapData mData = ((MapData*)body->dArr)[i];

			// TODO: filesystem 이용하여 한번에 넣도록
			result = m_MySQL->CallSaveMapData(
				uID.substr(0, 10), 
				dID.substr(0, 10), 
				roomNum,
				mData.bType, 
				mData.posX, 
				mData.posY, 
				mData.posZ, 
				mData.rotate
			);
		}

		std::cout << "[ SQL Process ] SAVE_DUNGEON_BLOCK_DATA : " << result << std::endl;
	}
	break;

	case SQL_TYPE::GET_DUNGEON_LIST:
	{
		GetDungeonListBody* body = (GetDungeonListBody*)sPkt.sBody;

		short numToGet = body->numToGet;
		std::cout << "[ SQL ] numToGet " << numToGet << std::endl;
		auto result = m_MySQL->CallGetDungeonList(numToGet);

		snPkt.pSession = sPkt.pSession;
		snPkt.packetID = (short)PACKETID::RES_GET_DUNGEON_LIST;
		snPkt.length = 62 * result.size() + PACKET_HEADER_SIZE + 2;

		char* ret = new char[snPkt.length];

		PacketHeader pHeader;
		pHeader.pID = PACKETID::RES_GET_DUNGEON_LIST;
		pHeader.bodySize = 62 * result.size() + 2;
		
		short dNum = result.size();

		memcpy(&ret[0], &pHeader, 4);
		memcpy(&ret[PACKET_HEADER_SIZE], &dNum, 2);

		for (int i = 0; i < result.size(); ++i)
		{
			memcpy(&ret[2 + 62 * i + PACKET_HEADER_SIZE], &result[i][0].c_str()[0], min(10, result[i][0].size()));
			memcpy(&ret[2 + 62 * i + PACKET_HEADER_SIZE + 10], &result[i][1].c_str()[0], min(10, result[i][1].size()));
			memcpy(&ret[2 + 62 * i + PACKET_HEADER_SIZE + 20], &result[i][2].c_str()[0], min(10, result[i][2].size()));
			memcpy(&ret[2 + 62 * i + PACKET_HEADER_SIZE + 30], &result[i][3].c_str()[0], min(30, result[i][3].size()));
			
			short roomnum = atoi(result[i][4].c_str());
			memcpy(&ret[2 + 62 * i + PACKET_HEADER_SIZE + 60], &roomnum, 2);
		}

		snPkt.pBuffer = ret;
	}
	break;

	case SQL_TYPE::GET_MAP_DATA:
	{
		GetMapDataPacket* body = (GetMapDataPacket*)sPkt.sBody;

		snPkt.pSession = sPkt.pSession;
		snPkt.packetID = (short)PACKETID::RES_GET_MAPDATA;

		auto result = m_MySQL->CallGetRoomNum(body->uID, body->dID);

		// 1개를 더 할당하여 0 인덱스는 데이터 보관에 사용
		// 포인터는 64bit에서는 8바이트, 여유롭게 보관 가능
		MapData** retmap = new MapData*[atoi(result[0][0].c_str()) +1];

		PacketHeader pHeader;
		pHeader.pID = PACKETID::RES_GET_MAPDATA;
		// 어차피 새로 생성해서 보낼 패킷들
		pHeader.bodySize = atoi(result[0][0].c_str());
		
		memcpy(&retmap[0], &pHeader, 4);

		for (int curRoomNum = 1; curRoomNum <= atoi(result[0][0].c_str()); ++curRoomNum)
		{
			auto blockData = m_MySQL->CallGetBlockDataByRoomNum(body->uID, body->dID, curRoomNum);
			MapData* mDataArr = new MapData[blockData.size() + 1];

			// 특별히 0번째 posX에는 사이즈를 넣어줌
			mDataArr[0].posX = blockData.size();
			for (int i = 1; i <= blockData.size(); ++i)
			{
				mDataArr[i].bType = atoi(blockData[i][3].c_str());
				mDataArr[i].posX = atoi(blockData[i][4].c_str());
				mDataArr[i].posY = atoi(blockData[i][5].c_str());
				mDataArr[i].posZ = atoi(blockData[i][6].c_str());
				mDataArr[i].rotate = atof(blockData[i][7].c_str());
			}

			retmap[curRoomNum] = mDataArr;
		}

		snPkt.length = PACKET_HEADER_SIZE + sizeof(retmap);
		//char* ret = new char[snPkt.length];

		
		// TODO: 포인터 주소만을 이용한 혼돈의 데이터 넘기기
		// safe 하지 않음
		snPkt.pBuffer = (char*)retmap;
	}
	break;

	default:
		break;
	}

	delete sPkt.sBody;
	SQLqueue.pop_front();
	return snPkt;
}

void DBManager::AddToSqlQueue(SQLPacket sPkt)
{
	SQLqueue.push_back(sPkt);
}

