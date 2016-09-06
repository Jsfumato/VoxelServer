#include "PacketManager.h"
#include <iostream>

PacketManager::PacketManager()
{
	m_DBManager = new DBManager();
	m_DBManager->InitMySQL();
}

PacketManager::~PacketManager()
{
}

ERROR_CODE PacketManager::CheckRecvPacket(Session * pSession)
{
	if (pSession == nullptr)
		return ERROR_CODE::INVALID_SESSION;

	PacketHeader* header = (PacketHeader*)&pSession->m_tmpRecvBuf[0];
	PacketBody* body = (PacketBody*)&pSession->m_tmpRecvBuf[PACKET_HEADER_SIZE];

	if (header->bodySize > MAX_BUFFER_SIZE)
		return ERROR_CODE::INVALID_REQ_PACKET;

	if (header->bodySize > pSession->m_tmpLen - PACKET_HEADER_SIZE)
	{
		pSession->recvComplete = false;
		return ERROR_CODE::RECV_INCOMPLETE;
	}


	// 조건 만족시 복사하여 queue로 던짐
	char* tmp = new char[header->bodySize + PACKET_HEADER_SIZE];
	std::copy(
		&pSession->m_tmpRecvBuf[0],
		&pSession->m_tmpRecvBuf[header->bodySize + PACKET_HEADER_SIZE],
		&tmp[0]
	);

	header = (PacketHeader*)&tmp[0];
	body = (PacketBody*)&tmp[PACKET_HEADER_SIZE];

	AppendToRecvPacketQueue(pSession, tmp, header->pID, header->bodySize + PACKET_HEADER_SIZE);


	// buffer 뒤에 있는 내용을 앞으로 당김
	std::copy(
		&pSession->m_tmpRecvBuf[header->bodySize + PACKET_HEADER_SIZE],
		&pSession->m_tmpRecvBuf[MAX_BUFFER_SIZE],
		&pSession->m_tmpRecvBuf[0]
	);

	pSession->m_tmpLen -= header->bodySize + PACKET_HEADER_SIZE;

	pSession->recvComplete = true;

	return ERROR_CODE::NONE;
}

void PacketManager::AppendToRecvPacketQueue(Session * pSession, char* buffer, PACKETID pID, const unsigned short dataSize)
{
	RecvPacket recvPacket;
	recvPacket.buffer = buffer;
	recvPacket.packetID = pID;
	recvPacket.pSession = pSession;
	recvPacket.recvSize = dataSize;

	RecvPacketQueue.push_back(recvPacket);
}

void PacketManager::AppendToSendPacketQueue(Session * pSession, char * buffer, PACKETID pID, const unsigned short dataSize)
{
	SendPacket sendPkt;
	sendPkt.pBuffer = buffer;
	sendPkt.packetID = pID;
	sendPkt.pSession = pSession;
	sendPkt.length = dataSize;

	SendPacketQueue.push_back(sendPkt);
}

void PacketManager::ProcessRecvPacketQueue()
{
	if (RecvPacketQueue.empty() == true)
		return;

	RecvPacket packet = RecvPacketQueue.front();
	RecvPacketQueue.pop_front();

	switch (packet.packetID)
	{
	case PACKETID::REQ_PING:
	{
		m_Mutex.lock();
		std::cout << "[ PING ] test ping from : " << inet_ntoa(packet.pSession->m_ClientAddress.sin_addr) << std::endl;
		m_Mutex.unlock();
	}
	break;

	case PACKETID::REQ_LOGIN:
	{
		short emailLength = 0;
		short passwordLength = 0;

		memcpy(&emailLength, &packet.buffer[4], 2);
		memcpy(&passwordLength, &packet.buffer[6], 2);

		char* email = new char[emailLength+1];
		char* password = new char[passwordLength+1];

		std::copy(&packet.buffer[8], &packet.buffer[8 + emailLength], &email[0]);
		std::copy(&packet.buffer[8 + emailLength], &packet.buffer[8 + emailLength + passwordLength], &password[0]);

		email[emailLength] = 0;
		password[passwordLength] = 0;

		m_Mutex.lock();
		std::cout << "[ REQ_LOGIN ] from : " << inet_ntoa(packet.pSession->m_ClientAddress.sin_addr) << std::endl;
		std::cout << "email : " << email << std::endl;
		std::cout << "password : " << password << std::endl;
		m_Mutex.unlock();

		SQLPacket sPkt;
		sPkt.sType = SQL_TYPE::GET_USER_INFO;
		sPkt.sBody = new GetUserInfoBody();
		auto body = (GetUserInfoBody*)sPkt.sBody;

		std::copy(&email[0], &email[emailLength + 1], &body->email[0]);
		std::copy(&password[0], &password[passwordLength + 1], &body->password[0]);
		body->emailLen = emailLength;
		body->passwordLen = passwordLength;

		sPkt.pSession = packet.pSession;

		m_DBManager->AddToSqlQueue(sPkt);
	}
	break;

	case PACKETID::REQ_REGISTER_USER:
	{
		short emailLength = 0;
		short passwordLength = 0;

		memcpy(&emailLength, &packet.buffer[4], 2);
		memcpy(&passwordLength, &packet.buffer[6], 2);

		char* email = new char[emailLength + 1];
		char* password = new char[passwordLength + 1];

		std::copy(&packet.buffer[8], &packet.buffer[8 + emailLength], &email[0]);
		std::copy(&packet.buffer[8 + emailLength], &packet.buffer[8 + emailLength + passwordLength], &password[0]);

		email[emailLength] = 0;
		password[passwordLength] = 0;

		m_Mutex.lock();
		std::cout << "[ REQ_REGISTER ] from : " << inet_ntoa(packet.pSession->m_ClientAddress.sin_addr) << std::endl;
		std::cout << "email : " << email << std::endl;
		std::cout << "password : " << password << std::endl;
		m_Mutex.unlock();

		SQLPacket sPkt;
		sPkt.sType = SQL_TYPE::REGISTER_USER;
		sPkt.sBody = new RegisterUserInfoBody();
		auto body = (RegisterUserInfoBody*)sPkt.sBody;

		std::copy(&email[0], &email[emailLength + 1], &body->email[0]);
		std::copy(&password[0], &password[passwordLength + 1], &body->password[0]);
		body->emailLen = emailLength;
		body->passwordLen = passwordLength;

		sPkt.pSession = packet.pSession;

		m_DBManager->AddToSqlQueue(sPkt);
	}
	break;

	case PACKETID::REQ_SAVE_MAP_NAME_INFO:
	{
		m_Mutex.lock();
		std::cout << "[ REQ_SAVE_MAP_NAME_INFO ] from : " << inet_ntoa(packet.pSession->m_ClientAddress.sin_addr) << std::endl;
		m_Mutex.unlock();

		SQLPacket sPkt;
		sPkt.sType = SQL_TYPE::SAVE_DUNGEON_NAME_INFO;
		sPkt.sBody = new SaveDungeonNameInfoBody();
		sPkt.pSession = (void*)packet.pSession;
		auto body = (SaveDungeonNameInfoBody*)sPkt.sBody;

		std::copy(&packet.buffer[PACKET_HEADER_SIZE], &packet.buffer[PACKET_HEADER_SIZE + 10], body->uID);
		std::copy(&packet.buffer[PACKET_HEADER_SIZE + 10], &packet.buffer[PACKET_HEADER_SIZE + 20], body->dName);
		std::copy(&packet.buffer[PACKET_HEADER_SIZE + 20], &packet.buffer[PACKET_HEADER_SIZE + 50], body->dInfo);
		memcpy(&body->roomNum, &packet.buffer[PACKET_HEADER_SIZE + 50], 2);

		m_DBManager->AddToSqlQueue(sPkt);
	}
	break;

	case PACKETID::REQ_SAVE_MAP_DATA:
	{
		m_Mutex.lock();
		std::cout << "[ REQ_SAVE_MAP_DATA ] from : " << inet_ntoa(packet.pSession->m_ClientAddress.sin_addr) << std::endl;
		m_Mutex.unlock();

		SQLPacket sPkt;
		sPkt.sType = SQL_TYPE::SAVE_DUNGEON_BLOCK_DATA;
		sPkt.sBody = new SaveDungeonMapDataBody();
		sPkt.pSession = (void*)packet.pSession;
		auto body = (SaveDungeonMapDataBody*)sPkt.sBody;
		
		memcpy(&body->roomNum, &packet.buffer[PACKET_HEADER_SIZE], 2);
		memcpy(&body->blockNum, &packet.buffer[PACKET_HEADER_SIZE + 2], 2);
		//std::copy(&packet.buffer[PACKET_HEADER_SIZE + 2], &packet.buffer[PACKET_HEADER_SIZE + 4], body->blockNum);
		std::copy(&packet.buffer[PACKET_HEADER_SIZE + 4], &packet.buffer[PACKET_HEADER_SIZE + 14], body->uID);
		std::copy(&packet.buffer[PACKET_HEADER_SIZE + 14], &packet.buffer[PACKET_HEADER_SIZE + 24], body->dID);

		m_Mutex.lock();
		std::cout << "[ REQ_SAVE_MAP_DATA ] RoomNum : " << body->roomNum << " | Get Blcoks : " << body->blockNum << " | From " << inet_ntoa(packet.pSession->m_ClientAddress.sin_addr) << std::endl;
		m_Mutex.unlock();

		MapData* mDataArr = new MapData[body->blockNum];

		for (int i = 0; i < body->blockNum; ++i)
		{
			memcpy(&mDataArr[i], &packet.buffer[PACKET_HEADER_SIZE + 24 + 12 * i], 12);
		}
		body->dArr = mDataArr;
		
		m_DBManager->AddToSqlQueue(sPkt);
	}
	break;

	case PACKETID::REQ_GET_DUNGEON_LIST:
	{
		m_Mutex.lock();
		std::cout << "[ REQ_GET_DUNGEON_LIST ] from : " << inet_ntoa(packet.pSession->m_ClientAddress.sin_addr) << std::endl;
		m_Mutex.unlock();

		SQLPacket sqlPkt;
		sqlPkt.sType = SQL_TYPE::GET_DUNGEON_LIST;
		sqlPkt.sBody = new GetDungeonListBody();
		sqlPkt.pSession = (void*)packet.pSession;

		auto body = (GetDungeonListBody*)sqlPkt.sBody;

		memcpy(&body->numToGet, &packet.buffer[PACKET_HEADER_SIZE], 2);

		m_DBManager->AddToSqlQueue(sqlPkt);
	}
	break;

	case PACKETID::REQ_GET_MAPDATA:
	{
		m_Mutex.lock();
		std::cout << "[ REQ_GET_MAPDATA ] from : " << inet_ntoa(packet.pSession->m_ClientAddress.sin_addr) << std::endl;
		m_Mutex.unlock();

		SQLPacket sqlPkt;
		sqlPkt.sType = SQL_TYPE::GET_MAP_DATA;
		sqlPkt.sBody = new GetMapDataPacket();
		sqlPkt.pSession = (void*)packet.pSession;

		auto body = (GetMapDataPacket*)sqlPkt.sBody;

		memcpy(&body->uID, &packet.buffer[PACKET_HEADER_SIZE], 10);
		memcpy(&body->dID, &packet.buffer[PACKET_HEADER_SIZE + 10], 10);

		m_DBManager->AddToSqlQueue(sqlPkt);
	}

	default:
		break;
	}

	// TODO: packet의 buffer는 동적으로 생성... 반드시 delete 시켜야 함
	delete[] packet.buffer;
}

void PacketManager::ProcessDBQueue()
{
	SQLtoNetPacket snPkt = m_DBManager->SQLProcess();

	PACKETID pID = (PACKETID)snPkt.packetID;

	if (pID == PACKETID::NONE)
		return;

	switch (pID)
	{
	case PACKETID::RES_GET_DUNGEON_LIST:
	case PACKETID::RES_LOGIN:
	case PACKETID::RES_REGISTER_USER:
	case PACKETID::RES_SAVE_MAP_NAME_INFO:
	{
		AppendToSendPacketQueue(
			(Session*)snPkt.pSession, 
			snPkt.pBuffer, 
			(PACKETID)snPkt.packetID, 
			snPkt.length
		);
	}
	break;

	case PACKETID::RES_GET_MAPDATA:
	{
		// 초기값 부여
		short roomNum = 1;
		memcpy(&roomNum, &snPkt.pBuffer[2], 2);

		MapData** arr_MapDataArr = (MapData**)snPkt.pBuffer;

		//// 해당 데이터의 0 인덱스에는 사이즈가 들어있음
		for (int i = 1; i <= roomNum; ++i)
		{
			short blockNum = arr_MapDataArr[i][0].posX;
			short totalLen = sizeof(MapData) * blockNum + PACKET_HEADER_SIZE + 2;
			char* buffer = new char[totalLen];

			PacketHeader pHeader;
			pHeader.pID = PACKETID::RES_GET_MAPDATA;
			pHeader.bodySize = sizeof(MapData) * blockNum + 2;

			memcpy(&buffer[0], &pHeader, 4);
			memcpy(&buffer[PACKET_HEADER_SIZE], &blockNum, 2);
			memcpy(&buffer[6], &arr_MapDataArr[i][1], sizeof(MapData) * blockNum);
			// std::copy(&arr_MapDataArr[i][1], &arr_MapDataArr[i][blockNum], &buffer[6]);

			AppendToSendPacketQueue(
				(Session*)snPkt.pSession,
				buffer,
				(PACKETID)snPkt.packetID,
				totalLen
			);
		}
	}
	break;

	default:
		break;
	}
}

void PacketManager::ProcessSendPacketQueue()
{
	bool result = false;
	SendPacket packet;

	m_Mutex.lock();
	if (SendPacketQueue.empty() == true)
		result = false;
	else
	{
		result = true;
		packet = SendPacketQueue.front();
		SendPacketQueue.pop_front();
	}
	m_Mutex.unlock();

	if (result == false)
		return;

	Session* pSession = (Session*)packet.pSession;

	// WSABUF 에 추가 및 전송할 길이 설정
	std::copy(
		&packet.pBuffer[0],
		&packet.pBuffer[packet.length],
		&pSession->m_SendContext->wsaBuffer.buf[0]
	);
	//pSession->m_SendContext->wsaBuffer.len = packet.length;

	DWORD dwSendBytes = 0;
	DWORD dwFlags = 0;
	
	int nReturn = WSASend(pSession->m_ClientSock, &pSession->m_SendContext->wsaBuffer, 1, 
		&dwSendBytes, dwFlags, &pSession->m_SendContext->overlapped, NULL);

	while (nReturn == SOCKET_ERROR)
	{
		if (WSAGetLastError() != ERROR_IO_PENDING)
		{
			// IO 진행 중
			// TODO:
			// socket ERROR!!!!!!
		}
		//TODO: ERROR_IO_PENDING 인 경우 send buffer 관리
		else
		{
			DWORD transferedDataSize;
			bool result = WSAGetOverlappedResult(pSession->m_ClientSock,
				&pSession->m_SendContext->overlapped, &transferedDataSize,
				FALSE, NULL);

			if (result == false)
			{
				// SOCKET ERROR
			}

			std::copy(
				&pSession->m_SendContext->wsaBuffer.buf[transferedDataSize],
				&pSession->m_SendContext->wsaBuffer.buf[MAX_BUFFER_SIZE],
				&pSession->m_SendContext->wsaBuffer.buf[0]
			);
			//pSession->m_SendContext->wsaBuffer.len -= transferedDataSize;

			// 다시 나머지 부분을 이어서 전송
			nReturn = WSASend(pSession->m_ClientSock, &pSession->m_SendContext->wsaBuffer, 1,
				&dwSendBytes, dwFlags, &pSession->m_SendContext->overlapped, NULL);
		}
	}

	// packet의 buffer는 동적으로 생성... 반드시 delete 시켜야 함
	delete[] packet.pBuffer;
}
