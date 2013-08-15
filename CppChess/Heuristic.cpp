#include "stdafx.h"
#include "Heuristic.h"

namespace
{
	const int contempt = -20;
	const int on_move = 5;
}

CHeuristic::CHeuristic(CBoard& b)
	: b(b)
{
}

CHeuristicParameters CHeuristic::params = CHeuristicParameters();

/*s*/ int CHeuristic::piece_value(chess::PIECE piece)
{
	return params.get_value(piece);
}

int CHeuristic::nonterminal_value()
{
	// material count
	int mat_count = 0;
	for (int side=0; side<2; side++)
	{
		int mult = side == b.side_on_move() ? 1 : -1;
		for (int piece=1; piece<7; piece++)
		{
			const CPiece p = CPiece(chess::SIDE(side), chess::PIECE(piece));
			const auto & pc = b.piece_table()[p];

			for (auto sq : pc)
				mat_count += params.get_value(p, CBoard::index(sq)) * mult;
			// 2 bishop bonus
			if (p.piece() == chess::BISHOP && pc.size() > 1)
			{
				mat_count += 25 * mult;
			}

		}
	}

	// central control todo

	// mobility
	//int mobility = lm.size();

	//TODO
	return mat_count + on_move;
}
int CHeuristic::value()
{
	auto lm = b.legal_moves();

	if (!lm.size())
	{
		// checkmated or draw
		if (b.is_check())
		{
			// checkmated
			return -10000;
		}
		else
		{
			return -contempt; // drawn position
		}
	}

	return nonterminal_value();

}


CHeuristic::~CHeuristic(void)
{
}
