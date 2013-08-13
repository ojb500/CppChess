#include "stdafx.h"
#include "Heuristic.h"


CHeuristic::CHeuristic(CBoard& b)
	: b(b)
{
}

CHeuristicParameters CHeuristic::params = CHeuristicParameters();

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
			return 0; // drawn position
		}
	}

	// material count
	int mat_count = 0;
	for (int side=0; side<2; side++)
	{
		for (int piece=1; piece<7; piece++)
		{
			const CPiece p(side == 0 ? chess::WHITE : chess::BLACK, chess::PIECE(piece));
			const auto & pc = b.piece_table()[p];

			int mult = (p.side() == b.side_on_move() ? 1 : -1);
			for (auto sq : pc)
				mat_count += params.GetValue(p, CBoard::index(sq)) * mult;
			// 2 bishop bonus
			if (p.piece() == chess::BISHOP && pc.size() > 1)
			{
				mat_count += 25 * mult;
			}
			
		}
	}

	// central control todo

	// mobility
	int mobility = lm.size();

	//TODO
	return (mobility / 2) + mat_count;

}


CHeuristic::~CHeuristic(void)
{
}
