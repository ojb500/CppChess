// CppChess.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "UciSession.h"

using namespace std;

int _tmain(int argc, _TCHAR* argv[])
{
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_EVERY_1024_DF);

	ofstream logfile;
	logfile.open("session.log", ofstream::out | ofstream::trunc);
	if (logfile.fail())
		exit(1);

	CUciSession uci(cin, cout, logfile);
	uci.listen();
	system("pause");
	logfile.close();
}

