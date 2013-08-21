#include "stdafx.h"
#include "UciSession.h"
#include "Board.h"
#include "Engine.h"
#include "Perft.h"
#include "PerftSuite.h"
#include "BratkoKopec.h"
#include <time.h>
#include "Tests.h"
#include <future>
#include "Version.h"

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
	, _cancelling(false)
{
	write_log_line(CVersion()());
}

void CUciSession::listen()
{
	string s;
	CBoard b;
	
	std::thread thr;
	

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
		else if (*j == "uci")
		{
			write_line("id name ojchess-cpp");
			write_line("uciok");
		}
		else if (*j == "isready")
		{
			write_line("readyok");
		}
		else if (*j == "board")
		{
			write_line("Current board:\r\n" + b.board());
		}
		else if (*j == "ucinewgame")
		{
			b = CBoard();
			write_line("readyok");
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
			CEngine e(*this, b);
			e.Perft(maxdepth);
		}
		else if (*j == "bk")
		{
			CBratkoKopec ps;
			write_line("completed");
		}
		else if (*j == "perftsuite")
		{
			CPerftSuite ps;
			write_line("completed");
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
			if (thr.joinable())
			{
				write_log_line("joining thread");
				thr.join();
			}

			_cancelling = false;

			// read the rest of the parameters (if any)
			CEngine::millisecs_t wtime(-1);
			CEngine::millisecs_t btime(-1);
			CEngine::millisecs_t winc(-1);
			CEngine::millisecs_t binc(-1);
			int movesToGo = -1;
			int maxDepth = -1;
			int maxNodes = -1;

			while (!j.at_end())
			{
				j++;
				// read eval parameters
				if (!j.at_end())
				{
					const std::string paramName = *j;
					j++;
					if (!j.at_end())
					{
						const int value = boost::lexical_cast<int>(*j);
						if (paramName == "wtime")
							wtime = CEngine::millisecs_t(value);
						else if (paramName == "btime")
							btime = CEngine::millisecs_t(value);
						else if (paramName == "winc")
							winc = CEngine::millisecs_t(value);
						else if (paramName == "binc")
							binc = CEngine::millisecs_t(value);
						else if (paramName == "movestogo")
							movesToGo = value;
						else if (paramName == "depth")
							maxDepth = value;
						else if (paramName == "nodes")
							maxNodes = value;
					}
				}
			}

			thr = std::thread([&, this] () {
				try
				{
					CEngine e(*this, b, wtime, winc, btime, binc, movesToGo, maxDepth, maxNodes);
					auto m = e.iterative_deepening();
					write_line("bestmove " + m.long_algebraic());
				}
				catch (CEngine::CExceptionCancelled)
				{
				}
				catch (...)
				{
					cout << "FUCK! ";// ##<< ex.what();
				}
				write_log_line("thread terminating normally");
			});
		}
		else if (*j == "stop")
		{
			if (thr.joinable())
			{
				_cancelling = true;
				thr.join();
			}
		}
	}
	ASSERT(false);
}
void CUciSession::write_line(std::string s)
{
	const std::string dt(DateTimeNow());
	_log << dt << " >> " << s << endl;
	_o << s << endl;
}

void CUciSession::write_log_line(std::string s)
{
	const std::string dt(DateTimeNow());
	_log << dt << " # " << s << endl;
	if (find(s.begin(), s.end(), '\n') == s.end())
		_o << "info string " << s << endl;
}


CUciSession::~CUciSession(void)
{
}

bool CUciSession::is_cancelling() const
{
	return _cancelling;
}