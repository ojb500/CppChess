// CppChess.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "UciSession.h"

using namespace std;

int _tmain(int argc, _TCHAR* argv[])
{
	ofstream logfile;
	logfile.open("session.log", ofstream::out | ofstream::trunc);
	if (logfile.fail())
		exit(1);

	CUciSession uci(cin, cout, logfile);
	uci.listen();
	system("pause");
	logfile.close();
}

