#pragma once
class CUciSession
{
public:
	CUciSession(std::istream & i, std::ostream & o, std::ostream & log);
	~CUciSession(void);

	void CmdUci();
	void CmdSetboard();
	void WriteLine(std::string s);

private:

	std::ostream _o;
	std::ostream _log;
};

