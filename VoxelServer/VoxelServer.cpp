// VoxelServer.cpp : �ܼ� ���� ���α׷��� ���� �������� �����մϴ�.
//

#include "stdafx.h"
#include "MainServer.h"

int main()
{
	MainServer theServer;
	bool result = theServer.InitServer();
	if (result == false)
		return 0;

	char input;
	while (input = getchar())
	{
		if(input == 'Q')
			theServer.EndServer();
	}

    return 0;
}	