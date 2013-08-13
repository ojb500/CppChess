#include "stdafx.h"
#include "UciSession.h"
#include "Board.h"
#include "Engine.h"
#include "Perft.h"
#include <time.h>
#include "Tests.h"


using namespace std;

namespace
{
	enum COMMAND
	{
		C_Noop,
		C_Go,
		C_SetBoard,
		C_Uci,
	};

	static const char *DAY_NAMES[] =
	{ "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	static const char *MONTH_NAMES[] =
	{ "Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

	char *DateTimeNow()
	{
		const int TIME_LEN = 10;
		time_t t;
		struct tm tm;
		char * buf = (char *) malloc(TIME_LEN+1);

		time(&t);
		gmtime_s(&tm, &t);

		strftime(buf, TIME_LEN+1, "[%H:%M:%S]", &tm);
		return buf;
	}
}

CUciSession::CUciSession(std::istream & i, std::ostream & o, std::ostream & log)
	: _i(i)
	, _o(o.rdbuf())
	, _log(log.rdbuf())
{
}

void CUciSession::listen()
{
	string s;
	CBoard b;
	std::shared_ptr<CEngine> e = make_shared<CEngine>(*this);

	while (getline(_i, s, '\n'))
	{

		const std::string dt(DateTimeNow()); // TODO EVIL?!?!
		_log << dt << " << " << s << endl;

		boost::char_separator<char> sep(" ");
		boost::tokenizer<boost::char_separator<char>> tok(s, sep);

		auto j = tok.begin();

		if (*j == "quit")
		{
			exit(0);
		}
		if (*j == "uci")
		{
			WriteLine("id name ojchess-cpp");
			WriteLine("uciok");
		}
		else if (*j == "isready")
		{
			WriteLine("readyok");
		}
		else if (*j == "board")
		{
			WriteLine("Current board:\r\n" + b.board());
		}
		else if (*j == "ucinewgame")
		{
			e = make_shared<CEngine>(*this);
			b = CBoard();
			WriteLine("readyok");
		}
		else if (*j == "position")
		{
			j++;
			b = CBoard();

			if (*j == "fen")
			{
				j++;
				auto i = j;
				while (j != tok.end() && *j != "moves")
					j++;

				std::string fen = boost::algorithm::join(std::vector<std::string>(i, j), " ");
				b.set_fen_position(fen);
			}
			else if (*j == "startpos")
			{
				j++;
			}

			if (j != tok.end())
			{
				ASSERT(*j == "moves");
				j++; // moves
				while (j != tok.end())
				{
					CMove mv = CMove::FromString(*j++);
					auto lm = b.legal_moves();
					auto m = find_if(lm.cbegin(), lm.cend(), [&](const CMove & m)
					{
						return m.from() == mv.from() && m.to() == mv.to();
					});
					if (m != lm.cend())
					{
						b.make_move(*m);
					}
					else
						ASSERT(false);
				}
			}
			e->set_position(b);
		}
		else if (*j == "perft")
		{
			++j;
			int maxdepth = 4;
			if (!j.at_end())
			{
				maxdepth = atoi(j->c_str());
				++j;
			}
			e->Perft(maxdepth);
		}
		else if (*j == "divide")
		{
			++j;
			int maxdepth = 5;
			if (!j.at_end())
			{
				maxdepth = atoi(j->c_str());
				++j;
			}
			CPerft perft(b);
			perft.Divide(maxdepth);
		}
		else if (*j == "tests")
		{
			CTests tests;
		}
		else if (*j == "go")
		{
			auto m = e->Think();

			WriteLine("bestmove " + m.long_algebraic());
			// TODO do stuff with engine
		}
	}
}
void CUciSession::WriteLine(std::string s)
{
	const std::string dt(DateTimeNow()); // TODO EVIL?!?!
	_log << dt << " >> " << s << endl;
	_o << s << endl;
}

void CUciSession::WriteLogLine(std::string s)
{
	const std::string dt(DateTimeNow()); // TODO EVIL?!?!
	_log << dt << " # " << s << endl;
}


CUciSession::~CUciSession(void)
{
}
