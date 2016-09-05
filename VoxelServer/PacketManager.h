#pragma once
#include <deque>
#include <mutex>

#include "Packet.h"
#include "SessionManager.h"
#include "ErrorCode.h"
#include "DBManager.h"

struct RecvPacket
{
	Session*	pSession;
	PACKETID	packetID;
	char*		buffer;
	unsigned short	recvSize;
};

struct SendPacket
{
	Session*	pSession;
	PACKETID	packetID;
	char*		pBuffer;
	unsigned short length;
};

class PacketManager
{
public:
	PacketManager();
	~PacketManager();

	ERROR_CODE CheckRecvPacket(Session * pSession);
	void AppendToRecvPacketQueue(Session * pSession, char * buffer, PACKETID pID, const unsigned short dataSize);
	void AppendToSendPacketQueue(Session * pSession, char * buffer, PACKETID pID, const unsigned short dataSize);

	void ProcessRecvPacketQueue();
	void ProcessDBQueue();
	void ProcessSendPacketQueue();

private:
	DBManager*		m_DBManager;

	std::deque<RecvPacket> RecvPacketQueue;
	std::deque<SendPacket> SendPacketQueue;
	std::mutex m_Mutex;
};

