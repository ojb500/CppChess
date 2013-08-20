#include "stdafx.h"
#include "Version.h"

const std::string CVersion::operator()()
{
	std::stringstream ss;
	ss << "CppChess 0.1, ";
	ss << __DATE__;
	ss << ", ";
	ss << __TIME__;
	return ss.str();
};