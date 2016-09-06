#include <iostream>
//#include <fstream>
#include <string>
//#include <clocale>
//#include <codecvt>

#include "format.h"
#include "OdbcLib.h"

Odbc* Odbc::_Inst = nullptr;

Odbc::Odbc()
	:_hEnv(nullptr), _hDbc(nullptr), _hStmt(nullptr)
{}

Odbc::~Odbc()
{
	_Inst = nullptr;
}

Odbc* Odbc::GetInstance()
{
	if (_Inst == nullptr)
		_Inst = new Odbc();
	else
		return _Inst;
}

bool Odbc::Connect(wchar_t* odbcName, wchar_t* mysqlId, wchar_t* password)
{
	int ret;

	ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &_hEnv);
	ret = SQLSetEnvAttr(_hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER);
	ret = SQLAllocHandle(SQL_HANDLE_DBC, _hEnv, &_hDbc);

	ret = SQLConnect(
		_hDbc,
		(SQLWCHAR*)odbcName, SQL_NTS,
		(SQLWCHAR*)mysqlId, SQL_NTS,
		(SQLWCHAR*)password, SQL_NTS
	);

	if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
	{
		_IsConnect = true;
		ret = SQLAllocHandle(SQL_HANDLE_STMT, _hDbc, &_hStmt);
		return true;
	}
	else
		return false;
}

void Odbc::DisConnect()
{
	if (_hStmt != nullptr)
		SQLFreeHandle(SQL_HANDLE_STMT, _hStmt);
	if (_hDbc != nullptr)
		SQLDisconnect(_hDbc);
	if (_hDbc != nullptr)
		SQLFreeHandle(SQL_HANDLE_DBC, _hDbc);
	if (_hEnv != nullptr)
		SQLFreeHandle(SQL_HANDLE_ENV, _hEnv);
	_IsConnect = false;
}


// http://stackoverflow.com/questions/17156998/c11-variadic-templates-return-tuple-from-variable-list-of-vectors
// http://stackoverflow.com/questions/20407753/can-c11-parameter-packs-be-used-outside-templates
//template<typename... Ts>

std::vector<std::vector<std::string>> Odbc::SelectFrom(std::string dbName, std::string tableName)
{
	//auto recordType = std::vector<std::string>();
	std::vector<std::vector<std::string>> retVector = std::vector<std::vector<std::string>>();

	if (!_IsConnect || _Inst == nullptr)
		return retVector;

	std::string query = fmt::format("SELECT * FROM {0}.{1}", dbName, tableName);
	
	SQLCHAR* sql = (SQLCHAR*)query.c_str();

	int ret = SQLAllocHandle(SQL_HANDLE_STMT, _hDbc, &_hStmt);
	ret = SQLExecDirectA(_hStmt, sql, SQL_NTS);

	if (ret == SQL_SUCCESS)
	{
		SQLLEN resultLen;
		SQLSMALLINT columns;

		while (true)
		{
			ret = SQLFetch(_hStmt);
			SQLNumResultCols(_hStmt, &columns);

			if (ret == SQL_NO_DATA || ret == SQL_ERROR || ret == SQL_SUCCESS_WITH_INFO)
				return retVector;

			//			SELECT
			if (ret == SQL_SUCCESS)
			{
				auto record = std::vector<std::string>();
				for (int i = 1; i <= columns; ++i)
				{
					char strResult[200];
					SQLGetData(_hStmt, i, SQL_C_CHAR, strResult, 200, &resultLen);
					record.push_back(strResult);
				}
				retVector.push_back(record);
			}
		}
		return retVector;
	}
}

std::vector<std::vector<std::string>> Odbc::CallSaveDungeonNameInfo(std::string uID, std::string dName, std::string dInfo, int hasRoomNum)
{
	std::vector<std::vector<std::string>> retVector = std::vector<std::vector<std::string>>();

	if (!_IsConnect || _Inst == nullptr)
		return retVector;

	std::wstring query = fmt::format(L"call SaveDungeonNameInfo('{0}', '{1}', '{2}', {3})", uID, dName, dInfo, hasRoomNum);

	SQLWCHAR* sql = (SQLWCHAR*)query.c_str();
	int ret = SQLAllocHandle(SQL_HANDLE_STMT, _hDbc, &_hStmt);
	ret = SQLExecDirect(_hStmt, sql, SQL_NTS);

	if (ret == SQL_SUCCESS)
	{
		SQLLEN resultLen;
		SQLSMALLINT columns;

		while (true)
		{
			ret = SQLFetch(_hStmt);
			SQLNumResultCols(_hStmt, &columns);

			if (ret == SQL_NO_DATA || ret == SQL_ERROR || ret == SQL_SUCCESS_WITH_INFO)
				return retVector;

			//			SELECT
			if (ret == SQL_SUCCESS)
			{
				auto record = std::vector<std::string>();
				for (int i = 1; i <= columns; ++i)
				{
					char strResult[200];
					SQLGetData(_hStmt, i, SQL_C_CHAR, strResult, 200, &resultLen);
					record.push_back(strResult);
				}
				retVector.push_back(record);
			}
		}
		return retVector;
	}
}

bool Odbc::CallSaveMapData(std::string uID, std::string dID, short roomNum, short bType, short posX, short posY, short posZ, float rotate)
{
	if (!_IsConnect || _Inst == nullptr)
		return false;

	std::wstring query = fmt::format(L"call SaveMapData('{0}', '{1}', {2}, {3}, {4}, {5}, {6}, {7})", uID, dID, roomNum, bType, posX, posY, posZ, rotate);

	SQLWCHAR* sql = (SQLWCHAR*)query.c_str();
	int ret = SQLAllocHandle(SQL_HANDLE_STMT, _hDbc, &_hStmt);
	ret = SQLExecDirect(_hStmt, sql, SQL_NTS);

	if (ret == SQL_SUCCESS)
		return true;
	else
		return false;
}

std::vector<std::vector<std::string>> Odbc::CallGetDungeonList(short num)
{
	std::vector<std::vector<std::string>> retVector = std::vector<std::vector<std::string>>();

	if (!_IsConnect || _Inst == nullptr)
		return retVector;

	std::wstring query = fmt::format(L"call GetDungeonList({0})", num);

	SQLWCHAR* sql = (SQLWCHAR*)query.c_str();
	int ret = SQLAllocHandle(SQL_HANDLE_STMT, _hDbc, &_hStmt);
	ret = SQLExecDirect(_hStmt, sql, SQL_NTS);

	if (ret == SQL_SUCCESS)
	{
		SQLLEN resultLen;
		SQLSMALLINT columns;

		while (true)
		{
			ret = SQLFetch(_hStmt);
			SQLNumResultCols(_hStmt, &columns);

			if (ret == SQL_NO_DATA || ret == SQL_ERROR || ret == SQL_SUCCESS_WITH_INFO)
				return retVector;

			//			SELECT
			if (ret == SQL_SUCCESS)
			{
				auto record = std::vector<std::string>();
				for (int i = 1; i <= columns; ++i)
				{
					char strResult[200];
					SQLGetData(_hStmt, i, SQL_C_CHAR, strResult, 200, &resultLen);
					record.push_back(strResult);
				}
				retVector.push_back(record);
			}
		}
		return retVector;
	}
}


std::vector<std::vector<std::string>> Odbc::CallGetUserData(std::string email, std::string pw)
{
	std::vector<std::vector<std::string>> retVector = std::vector<std::vector<std::string>>();

	if (!_IsConnect || _Inst == nullptr)
		return retVector;

	std::wstring query = fmt::format(L"call GetUserData('{0}', '{1}')", email, pw);

	SQLWCHAR* sql = (SQLWCHAR*)query.c_str();
	int ret = SQLAllocHandle(SQL_HANDLE_STMT, _hDbc, &_hStmt);
	ret = SQLExecDirect(_hStmt, sql, SQL_NTS);

	if (ret == SQL_SUCCESS)
	{
		SQLLEN resultLen;
		SQLSMALLINT columns;

		while (true)
		{
			ret = SQLFetch(_hStmt);
			SQLNumResultCols(_hStmt, &columns);

			if (ret == SQL_NO_DATA || ret == SQL_ERROR || ret == SQL_SUCCESS_WITH_INFO)
				return retVector;

			//			SELECT
			if (ret == SQL_SUCCESS)
			{
				auto record = std::vector<std::string>();
				for (int i = 1; i <= columns; ++i)
				{
					char strResult[200];
					SQLGetData(_hStmt, i, SQL_C_CHAR, strResult, 200, &resultLen);
					record.push_back(strResult);
				}
				retVector.push_back(record);
			}
		}
		return retVector;
	}
}

std::vector<std::vector<std::string>> Odbc::CallRegisterUser(std::string email, std::string pw)
{
	std::vector<std::vector<std::string>> retVector = std::vector<std::vector<std::string>>();

	if (!_IsConnect || _Inst == nullptr)
		return retVector;

	std::wstring query = fmt::format(L"call RegisterUser('{0}', '{1}')", email, pw);

	SQLWCHAR* sql = (SQLWCHAR*)query.c_str();
	int ret = SQLAllocHandle(SQL_HANDLE_STMT, _hDbc, &_hStmt);
	ret = SQLExecDirect(_hStmt, sql, SQL_NTS);

	if (ret == SQL_SUCCESS)
	{
		SQLLEN resultLen;
		SQLSMALLINT columns;

		while (true)
		{
			ret = SQLFetch(_hStmt);
			SQLNumResultCols(_hStmt, &columns);

			if (ret == SQL_NO_DATA || ret == SQL_ERROR || ret == SQL_SUCCESS_WITH_INFO)
				return retVector;

			//			SELECT
			if (ret == SQL_SUCCESS)
			{
				auto record = std::vector<std::string>();
				for (int i = 1; i <= columns; ++i)
				{
					char strResult[200];
					SQLGetData(_hStmt, i, SQL_C_CHAR, strResult, 200, &resultLen);
					record.push_back(strResult);
				}
				retVector.push_back(record);
			}
		}
		return retVector;
	}
}

std::vector<std::vector<std::string>> Odbc::CallGetRoomNum(std::string uID, std::string dID)
{
	std::vector<std::vector<std::string>> retVector = std::vector<std::vector<std::string>>();

	if (!_IsConnect || _Inst == nullptr)
		return retVector;

	std::wstring query = fmt::format(L"call GetMapTileNum('{0}', '{1}')", uID, dID);

	SQLWCHAR* sql = (SQLWCHAR*)query.c_str();
	int ret = SQLAllocHandle(SQL_HANDLE_STMT, _hDbc, &_hStmt);
	ret = SQLExecDirect(_hStmt, sql, SQL_NTS);

	if (ret == SQL_SUCCESS)
	{
		SQLLEN resultLen;
		SQLSMALLINT columns;

		while (true)
		{
			ret = SQLFetch(_hStmt);
			SQLNumResultCols(_hStmt, &columns);

			if (ret == SQL_NO_DATA || ret == SQL_ERROR || ret == SQL_SUCCESS_WITH_INFO)
				return retVector;

			//			SELECT
			if (ret == SQL_SUCCESS)
			{
				auto record = std::vector<std::string>();
				for (int i = 1; i <= columns; ++i)
				{
					char strResult[200];
					SQLGetData(_hStmt, i, SQL_C_CHAR, strResult, 200, &resultLen);
					record.push_back(strResult);
				}
				retVector.push_back(record);
			}
		}
		return retVector;
	}
}

std::vector<std::vector<std::string>> Odbc::CallGetBlockDataByRoomNum(std::string uID, std::string dID, int roomNumber)
{
	std::vector<std::vector<std::string>> retVector = std::vector<std::vector<std::string>>();

	if (!_IsConnect || _Inst == nullptr)
		return retVector;

	std::wstring query = fmt::format(L"call GetBlockDataByRoomNum('{0}', '{1}', {2})", uID, dID, roomNumber);

	SQLWCHAR* sql = (SQLWCHAR*)query.c_str();
	int ret = SQLAllocHandle(SQL_HANDLE_STMT, _hDbc, &_hStmt);
	ret = SQLExecDirect(_hStmt, sql, SQL_NTS);

	if (ret == SQL_SUCCESS)
	{
		SQLLEN resultLen;
		SQLSMALLINT columns;

		while (true)
		{
			ret = SQLFetch(_hStmt);
			SQLNumResultCols(_hStmt, &columns);

			if (ret == SQL_NO_DATA || ret == SQL_ERROR || ret == SQL_SUCCESS_WITH_INFO)
				return retVector;

			//			SELECT
			if (ret == SQL_SUCCESS)
			{
				auto record = std::vector<std::string>();
				for (int i = 1; i <= columns; ++i)
				{
					char strResult[200];
					SQLGetData(_hStmt, i, SQL_C_CHAR, strResult, 200, &resultLen);
					record.push_back(strResult);
				}
				retVector.push_back(record);
			}
		}
		return retVector;
	}
}



