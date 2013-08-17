#include "stdafx.h"
#include "HeuristicParameters.h"

namespace
{
	const int base_values[] = { 0, // NOTHING = 0,
		100, // PAWN = 1,
		400, // KNIGHT = 2,
		400, // BISHOP = 3,
		600, // ROOK = 4,
		1200, // QUEEN = 5,
		100000, // KING = 6,
	};
}

CHeuristicParameters::CHeuristicParameters(void)
	: _base_values(base_values, base_values + _countof(base_values))
	, _piece_squares(chess::PIECE_LAST + 1)
{
	std::ifstream params;
	params.open("Params.txt", std::ifstream::in);
	if (params.is_open())
	{
		for (int i=chess::PIECE_FIRST; i<=chess::PIECE_LAST; i++)
		{
			std::string line;
			int square = chess::A8;
			while (! params.eof() && square <= chess::H1)
			{
				getline(params, line);
				if (line[0] == '#')
					continue;
				boost::char_separator<char> sep("\t ,");
				boost::tokenizer<boost::char_separator<char>> toks(line, sep);
				for (const auto tok : toks)
				{
					int bonus = atoi(tok.c_str());
					_piece_squares[i].push_back(bonus);
					square++;
				}
			}
		}
	}
	else
	{
		ASSERT(false);
	}
}

int CHeuristicParameters::get_value(chess::PIECE piece)
{
	return _base_values[piece];
}

int CHeuristicParameters::get_value(CPiece piece, chess::SQUARES sq)
{
	if (!piece)
		return 0;
	if (piece.side() == chess::BLACK)
		sq = static_cast<chess::SQUARES>(chess::H1 - sq);
	return _base_values[piece.piece()] + _piece_squares[piece.piece()][sq];
}

CHeuristicParameters::~CHeuristicParameters(void)
{
}
