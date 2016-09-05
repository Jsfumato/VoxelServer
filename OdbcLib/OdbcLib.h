#pragma once
#include <Windows.h>
#include <vector>
#include "sql.h"
#include "sqlext.h"

class Odbc
{
public:
	static Odbc* _Inst;
	static Odbc* Odbc::GetInstance();

	bool	IsConnect() { return _IsConnect; }

	bool	Connect(wchar_t* odbcName, wchar_t* mysqlId, wchar_t* password);
	void	DisConnect();

	std::vector<std::vector<std::string>> SelectFrom(std::string dbName, std::string tableName);
	
	bool CallSaveDungeonNameInfo(std::string uID, std::string dName, std::string dInfo, int hasRoomNum);
	bool CallSaveMapData(std::string uID, std::string dID, short roomNum, short bType, short posX, short posY, short posZ, float rotate);

	std::vector<std::vector<std::string>> CallGetDungeonList(short num);
	std::vector<std::vector<std::string>> CallGetUserData(std::string email, std::string ps);
	std::vector<std::vector<std::string>> CallRegisterUser(std::string email, std::string pw);

private:
	Odbc();
	~Odbc();

	bool		_IsConnect = false;
	SQLHENV		_hEnv;
	SQLHDBC		_hDbc;
	SQLHSTMT	_hStmt;
};