#include "MainServer.h"
#include <iostream>

MainServer::MainServer()
{

}

MainServer::~MainServer()
{
}


unsigned int WINAPI MainServer::AcceptThread(LPVOID parameter)
{
	MainServer *pThread = (MainServer *)parameter;
	if (pThread == NULL)
		return 0;

	pThread->_AcceptThread();
	return 0;
}
unsigned int WINAPI MainServer::WorkerThread(LPVOID parameter)
{
	MainServer *pThread = (MainServer *)parameter;
	if (pThread == NULL)
		return 0;

	pThread->_WorkerThread();
	return 0;
}
unsigned int WINAPI MainServer::SendThread(LPVOID parameter)
{
	MainServer *pThread = (MainServer *)parameter;
	if (pThread == NULL)
		return 0;

	pThread->_SendThread();
	return 0;
}

unsigned int WINAPI MainServer::SqlThread(LPVOID parameter)
{
	MainServer *pThread = (MainServer *)parameter;
	if (pThread == NULL)
		return 0;

	pThread->_SqlThread();
	return 0;
}


bool MainServer::InitServer()
{
	// Manage Session
	m_SessionManager = new SessionManager();
	m_SessionManager->InitSessionPool();
	m_PacketManager = new PacketManager();

	// Init Socket
	if (WSAStartup(MAKEWORD(2, 2), &m_wsaData) != 0)
		return false;
	
	m_hCompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	GetSystemInfo(&m_SystemInfo);

	m_hServerSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	ZeroMemory(&m_ServerAddress, sizeof(m_ServerAddress));

	m_ServerAddress.sin_family = AF_INET;
	m_ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	m_ServerAddress.sin_port = htons(atoi("8777"));

	bind(m_hServerSocket, (SOCKADDR*)&m_ServerAddress, sizeof(m_ServerAddress));
	listen(m_hServerSocket, SOMAXCONN);

	isInit = true;
	isRun = true;

	if (isInit == false)
		return false;

	for (int i = 0; i < m_SystemInfo.dwNumberOfProcessors *2; ++i)
	{
		auto hThread = (HANDLE)_beginthreadex(NULL, 0, WorkerThread, (LPVOID)this, 0, NULL);
		threadHandleDeque.push_back(hThread);
	}

	auto hAccept = (HANDLE)_beginthreadex(NULL, 0, AcceptThread, (LPVOID)this, 0, NULL);
	auto hSend = (HANDLE)_beginthreadex(NULL, 0, SendThread, (LPVOID)this, 0, NULL);
	auto hSQL = (HANDLE)_beginthreadex(NULL, 0, SqlThread, (LPVOID)this, 0, NULL);

	threadHandleDeque.push_back(hAccept);
	threadHandleDeque.push_back(hSend);
	threadHandleDeque.push_back(hSQL);

	std::cout << "[ Info ] Server Start..." << std::endl;
	return true;
}

void MainServer::Run()
{

}

bool MainServer::EndServer()
{
	delete m_SessionManager;
	delete m_PacketManager;

	while (threadHandleDeque.empty() == false)
	{
		auto hThread = threadHandleDeque.front();
		CloseHandle(hThread);
		threadHandleDeque.pop_front();
	}

	std::cout << "[ Info ] Server Quit..." << std::endl;
	return true;
}

void MainServer::_AcceptThread()
{
	while (true)
	{
		SOCKET hClntSock;
		SOCKADDR_IN clientAddress;
		int addrLen = sizeof(clientAddress);
	
		// �⺻������ accept �Լ��� ������ ���� ������ ���
		hClntSock = accept(m_hServerSocket, (SOCKADDR*)&clientAddress, &addrLen);
	
		if (hClntSock == INVALID_SOCKET)
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK)
				continue;
			else
			{
				// TODO: ODBC ���� ���������� accept �Լ� ȣ���ϴ� ���� �ݵ�� �ذ��ؾ� ��
				/*if (clientAddress.sin_family == 52428)
					continue;
*/
				std::cout << "[ Accept ERROR ] ���� ���� : " << inet_ntoa(clientAddress.sin_addr) << std::endl;
				continue;
			}
		}

		auto pSession = m_SessionManager->GetEmptySession();
		m_SessionManager->AllocClientToSession(pSession, hClntSock, clientAddress);

		// TODO: pSession Ȯ��
		CreateIoCompletionPort((HANDLE)hClntSock, m_hCompletionPort, (ULONG_PTR)pSession, 0);
	
		std::cout <<  "[ Accept ] : " << inet_ntoa(clientAddress.sin_addr) << " " << pSession << std::endl;

		DWORD recvBytes = 0;
		DWORD flags = 0;
		// WSARecv �Լ��� ȣ���Ͽ� �ش� ���Ͽ� ���� ��Ű��
		WSARecv(hClntSock, &(pSession->m_RecvContext->wsaBuffer), 1, 
			&recvBytes, &flags, &(pSession->m_RecvContext->overlapped), NULL);
	}

	return;
}

void MainServer::_WorkerThread()
{

	Session* pSession;
	Context* pContext;


	while (true)
	{
		DWORD bytes = 0;

		auto result = GetQueuedCompletionStatus(m_hCompletionPort, &bytes,
			(PULONG_PTR)&pSession, (LPOVERLAPPED*)&pContext, INFINITE);

		if (result == 0 || bytes <= 0)
		{
			// recv �Ǵ� send�� ũ�Ⱑ 0 ����?
			// ERROR ó��
			std::cout << "[ Close Session ] : " << inet_ntoa(pSession->m_ClientAddress.sin_addr) << std::endl;
			m_SessionManager->CloseSession(pSession);
			continue;
		}

		if (pContext == pSession->m_RecvContext)
		{
			// pSession->m_RecvContext->wsaBuffer.len = bytes;
			
			char* tmp = new char[bytes]();

			if (pSession->recvComplete == true)
				pSession->m_tmpLen = 0;

			std::copy(
				&pSession->m_RecvContext->wsaBuffer.buf[0],
				&pSession->m_RecvContext->wsaBuffer.buf[bytes],
				&pSession->m_tmpRecvBuf[pSession->m_tmpLen]
			);

			pSession->m_tmpLen += bytes;
			m_PacketManager->CheckRecvPacket(pSession);

			// WSARecv �Լ��� ȣ���Ͽ� �ش� ���Ͽ� ���� ��Ű��
			DWORD flags = 0;
			WSARecv(pSession->m_ClientSock, &(pSession->m_RecvContext->wsaBuffer), 1,
				NULL, &flags, &(pSession->m_RecvContext->overlapped), NULL);
		}
		else if (pContext == pSession->m_SendContext)
		{
			// Send �� ó��
		}
		
		m_PacketManager->ProcessRecvPacketQueue();
	}

	return;
}

void MainServer::_SendThread()
{
	while(true)
		m_PacketManager->ProcessSendPacketQueue();

	return;
}

void MainServer::_SqlThread()
{
	while (true)
		m_PacketManager->ProcessDBQueue();

	return;
}