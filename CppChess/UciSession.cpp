#include "stdafx.h"
#include "UciSession.h"
#include "Board.h"
#include "Engine.h"

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
}

CUciSession::CUciSession(std::istream & i, std::ostream & o, std::ostream & log)
	: _o(o.rdbuf())
	, _log(log.rdbuf())
{
	string s;
	CBoard b;

	while (getline(i, s, '\n'))
	{
		_log << "<< " << s << endl;
		boost::tokenizer<> tok(s);

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
		else if (*j == "ucinewgame")
		{
			b = CBoard();
			WriteLine("readyok");
		}
		else if (*j == "position")
		{
			j++;
			if (*j == "startpos")
			{
				b = CBoard();
				j++; // moves
				if (*j == "moves")
				{
					j++;
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

						// TODO make the movage
					}
					continue;
				}
			}
		}
		else if (*j == "go")
		{
			CEngine e(*this, b);
			auto m = e.Think();

			WriteLine("bestmove " + m.long_algebraic());
			// TODO do stuff with engine
		}
		else
		{
			ASSERT(false);
		}

	}
}

void CUciSession::WriteLine(std::string s)
{
	_log << ">> " << s << endl;
	_o << s << endl;
}

CUciSession::~CUciSession(void)
{
}
