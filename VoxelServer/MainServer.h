#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <winsock2.h>
#include <windows.h>
#include <mutex>
#pragma comment(lib,"ws2_32.lib")

#include "SessionManager.h"
#include "PacketManager.h"

class MainServer
{
public:
	MainServer();
	~MainServer();

	bool InitServer();
	void Run();
	bool EndServer();

	bool isRun = false;

private:
	static unsigned int WINAPI AcceptThread(LPVOID parameter);
	static unsigned int WINAPI WorkerThread(LPVOID parameter);
	static unsigned int WINAPI SendThread(LPVOID parameter);
	static unsigned int WINAPI SqlThread(LPVOID parameter);

	void _WorkerThread();
	void _AcceptThread();
	void _SendThread();
	void _SqlThread();

private:

	WSADATA		m_wsaData;
	SYSTEM_INFO m_SystemInfo;
	SOCKADDR_IN m_ServerAddress;

	SOCKET		m_hServerSocket;
	HANDLE		m_hCompletionPort;

	SessionManager*	m_SessionManager;
	PacketManager*	m_PacketManager;

	std::mutex m_Mutex;
	bool isInit = false;

	std::deque<HANDLE> threadHandleDeque;
};