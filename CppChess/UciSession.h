#pragma once
class CUciSession
{
public:
	CUciSession(std::istream & i, std::ostream & o, std::ostream & log);
	~CUciSession(void);

	void WriteLine(std::string s);
	void WriteLogLine(std::string s);

	void listen();
private:
	std::istream & _i;

	std::ostream _o;
	std::ostream _log;
};

