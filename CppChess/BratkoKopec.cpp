#include "stdafx.h"
#include "BratkoKopec.h"
#include "Square.h"
#include "Engine.h"
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/null.hpp>

using namespace std;

CBratkoKopec::CBratkoKopec()
{
	std::ifstream params;
	params.open("BratkoKopec.epd.txt", std::ifstream::in);
	int passed = 0;
	int of = 0;
	if (params.is_open())
	{
		std::string line;
		int square = chess::A8;
		while (! params.eof())
		{
			getline(params, line);
			if (line[0] == '#')
				continue;

			of++;
			boost::char_separator<char> sep(";");
			boost::char_separator<char> sep2(" ");
			boost::tokenizer<boost::char_separator<char>> toks(line, sep);
			// 1st tok is fen
			auto tok = toks.begin();
			const std::string fen = *tok++;

			CBoard b;
			b.set_fen_position(fen);

			std::cout << b.board() << std::endl;

			// 2nd tok is bestmoves
			boost::iostreams::stream< boost::iostreams::null_sink > nullOstream( ( boost::iostreams::null_sink() ) );

			CUciSession s(cin, nullOstream, nullOstream);

			vector<std::string> bestMoves;
			{
				std::string bms = *tok++;
				boost::tokenizer<> toks(bms);
				auto moves = toks.begin();
				moves++;
				for_each(moves, toks.end(), [&](std::string move)
				{
					boost::algorithm::trim(move);
					bestMoves.push_back(move);
				});
			}



			std::cout << "expecting one of ";
			for (const auto bm : bestMoves)
				std::cout << bm << " ";
			std::cout << "...";
			CEngine eng(s);
			eng.set_position(b);
			const CMove move = eng.IterativeDeepening(chrono::milliseconds(20000));
			cout << b.san_name(move);

			if (find(bestMoves.begin(), bestMoves.end(), b.san_name(move)) != bestMoves.end())
			{
				cout << "!! " << endl;
				passed++;
				}
			else
			{
				cout << "?" << endl << "FEN:" << b.fen() << endl << endl;
			}
		}
		std::cout << "passed" << passed << " of " << of;
	}

}


CBratkoKopec::~CBratkoKopec(void)
{
}
