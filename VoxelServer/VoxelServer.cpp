// VoxelServer.cpp : 콘솔 응용 프로그램에 대한 진입점을 정의합니다.
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