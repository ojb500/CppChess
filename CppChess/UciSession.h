#pragma once
class CUciSession
{
public:
	CUciSession(std::istream & i, std::ostream & o, std::ostream & log);
	~CUciSession(void);

	void write_line(std::string s);
	void write_log_line(std::string s);

	void listen();

	bool is_cancelling() const;
private:
	std::istream & _i;

	std::ostream _o;
	std::ostream _log;

	bool _cancelling;
};

