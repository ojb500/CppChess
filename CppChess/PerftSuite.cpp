#include "stdafx.h"
#include "PerftSuite.h"
#include "Perft.h"

CPerftSuite::CPerftSuite(void)
{
	std::ifstream params;
	params.open("perftsuite.epd.txt", std::ifstream::in);
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

			std::cout << b.fen() << " ";

			int depth = 1;

			while (tok != toks.end())
			{
				boost::tokenizer<boost::char_separator<char>> toks2(*tok, sep2);
				auto tok2 = toks2.begin();
				tok2++;

				uint64_t i = std::stoull(*tok2);
				
				if (i > 100000000)
					break; // takes too long

				CPerft p(b);
				auto res = p.DoPerft(depth);

				if (res.nodes == i)
				{
					std::cout << ".";
				}
				else
				{
					std::cout << "WRONG (depth " << depth << ": " << res.nodes << " != expected " << i << ")" << std::endl;
					ASSERT(res.nodes == i);
					break;
				}
				depth++;
				tok++;
			}
		}
		std::cout << "completed";
	}
	
}


CPerftSuite::~CPerftSuite(void)
{
}
