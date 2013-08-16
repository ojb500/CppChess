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
	if (params.is_open())
	{
		std::string line;
		int square = chess::A8;
		while (! params.eof())
		{
			getline(params, line);
			if (line[0] == '#')
				continue;

			boost::char_separator<char> sep(";");
			boost::char_separator<char> sep2(" ");
			boost::tokenizer<boost::char_separator<char>> toks(line, sep);
			// 1st tok is fen
			auto tok = toks.begin();
			const std::string fen = *tok++;

			CBoard b;
			b.set_fen_position(fen);

			std::cout << b.board() << std::endl;

			// 2nd tok is bestmove
			boost::iostreams::stream< boost::iostreams::null_sink > nullOstream( ( boost::iostreams::null_sink() ) );

			CUciSession s(cin, nullOstream, nullOstream);
			const std::string bm = *tok++;

			CEngine eng(s);
			eng.set_position(b);
			cout << eng.IterativeDeepening(chrono::milliseconds(20000)).longer_algebraic() << " == " << bm << " ?";
		}
		std::cout << "completed";
	}

}


CBratkoKopec::~CBratkoKopec(void)
{
}
