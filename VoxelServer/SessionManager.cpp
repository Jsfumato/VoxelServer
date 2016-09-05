#include "SessionManager.h"


Context::~Context()
{
	delete buffer;
}

void Context::Clear()
{
	ZeroMemory(&overlapped, sizeof(overlapped));
	ZeroMemory(&wsaBuffer.buf, sizeof(MAX_BUFFER_SIZE));
	ZeroMemory(&buffer, sizeof(MAX_BUFFER_SIZE));
	wsaBuffer.len = MAX_BUFFER_SIZE;
}











Session::~Session()
{
	delete m_RecvContext;
	delete m_SendContext;
}

void Session::InitSession(SOCKET clientSock, SOCKADDR_IN clientAddress)
{
	m_ClientSock = clientSock;
	m_ClientAddress = clientAddress;

	m_RecvContext = new Context();
	m_RecvContext->wsaBuffer.buf = &m_RecvContext->buffer[0];
	m_RecvContext->wsaBuffer.len = MAX_BUFFER_SIZE;

	m_SendContext = new Context();
	m_SendContext->wsaBuffer.buf = m_RecvContext->buffer;
	m_SendContext->wsaBuffer.len = MAX_BUFFER_SIZE;

	isUsed = true;
}

void Session::ClearSession()
{
	if (m_ClientSock != INVALID_SOCKET)
	{
		closesocket(m_ClientSock);
		m_ClientSock = INVALID_SOCKET;
	}

	ZeroMemory(&m_ClientAddress, sizeof(m_ClientAddress));
	m_RecvContext->Clear();
	m_SendContext->Clear();

	isUsed = false;
}





SessionManager::SessionManager()
{
}

SessionManager::~SessionManager()
{
}

bool SessionManager::InitSessionPool()
{
	for (int i = 0; i < MAX_CLIENT_COUNT; ++i)
	{
		m_SessionPool.emplace_back();
	}

	if (m_SessionPool.size() < MAX_CLIENT_COUNT)
		return false;

	return true;
}

Session* SessionManager::GetEmptySession()
{
	Session* result = nullptr;

	m_Mutex.lock();
	for (auto& session : m_SessionPool)
	{
		if (session.IsUsed() == false)
		{
			result = &session;
			break;
		}
	}
	m_Mutex.unlock();

	return result;
}

bool SessionManager::AllocClientToSession(Session* pSession, SOCKET clientSocket, SOCKADDR_IN clientAddress)
{
	if (pSession == nullptr)
		return false;

	if (pSession->IsUsed() == true)
		return false;

	pSession->InitSession(clientSocket, clientAddress);

	return true;
}

void SessionManager::CloseSession(Session* pSession)
{
	pSession->ClearSession();
}

