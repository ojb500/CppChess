#include "stdafx.h"
#include "Heuristic.h"

namespace
{
	const int contempt = -20;
	const int on_move = 5;

	std::pair<unsigned char,unsigned char> pawn_structure(CBoard& b, chess::SIDE side)
	{
		unsigned char doubled = 0;
		unsigned char passed = 0;

		const CPiece ourp = CPiece(side, chess::PAWN);
		const CPiece theirp = CPiece(other_side(side), chess::PAWN);

		std::vector<std::vector<CBoard::INT_SQUARES>> ours(8);
		std::vector<std::vector<CBoard::INT_SQUARES>> theirs(8);

		{
			// ours
			const auto & pieces = b.piece_table()[ourp];
			for (const auto & sq : pieces)
			{
				ours[chess::square_file(CBoard::index(sq))].push_back(sq);
			}
		}
		{
			// theirs
			const auto & pieces = b.piece_table()[theirp];
			for (const auto & sq : pieces)
			{
				theirs[chess::square_file(CBoard::index(sq))].push_back(sq);
			}
		}

		for (int file=0; file<8; ++file)
		{
			const int direction = side == chess::WHITE ? -1 : 1;
			const auto & ourPawns = ours[file];
			const size_t num = ourPawns.size();
			if (num > 0)
			{
				// double penalty
				doubled += (num-1);

				const auto & otherPawns = theirs[file];

				for (const CBoard::INT_SQUARES pawn : ours[file])
				{
					const int signed_rank = (pawn >> 4) * direction;
					
					auto blocking = find_if(otherPawns.begin(), otherPawns.end(), [&](CBoard::INT_SQUARES s)
					{
						const int rk = (s >> 4) * direction;
						return rk > signed_rank;
					});
					if (blocking == otherPawns.end())
					{
						bool is_passed = true;
						if (file > 0)
						{
							// Check pawn at left
							auto otherPawns = theirs[file - 1];
							auto blocking = find_if(otherPawns.begin(), otherPawns.end(), [&](CBoard::INT_SQUARES s)
							{
								const int rk = (s >> 4) * direction;
								return rk > signed_rank;
							});
							is_passed &= (blocking == otherPawns.end());
						}
						if (is_passed && file < 7)
						{
							// Check pawn at right
							auto otherPawns = theirs[file + 1];
							auto blocking = find_if(otherPawns.begin(), otherPawns.end(), [&](CBoard::INT_SQUARES s)
							{
								const int rk = (s >> 4) * direction;
								return rk > signed_rank;
							});
							is_passed &= (blocking == otherPawns.end());
						}
						if (is_passed)
						{
							passed++;
						}
					}
				}
			}
		}
		return std::make_pair(passed, doubled);
	}
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
	const int stage = b.game_stage();

	int mat_count = 0;

	std::vector<unsigned char> pawnfiles(16);
	std::vector<unsigned char> rookfiles(16);

	for (int side=0; side<2; side++)
	{
		const int mult = side == b.side_on_move() ? 1 : -1;

		for (int piece=chess::PAWN; piece<7; piece++)
		{
			const CPiece p = CPiece(chess::SIDE(side), chess::PIECE(piece));
			const auto & pc = b.piece_table()[p];

			for (auto sq : pc)
			{
				mat_count += params.get_value(p, CBoard::index(sq), stage) * mult;
				if (p.piece() == chess::ROOK)
				{
					const size_t rookfile_idx = chess::square_file(CBoard::index(sq)) + (side*8);
					rookfiles[rookfile_idx]++;
				}
			}
			// 2 bishop bonus
			if (p.piece() == chess::BISHOP && pc.size() > 1)
			{
				mat_count += 25 * mult;
			}
		}
	}

	// passed pawn detection
	for (int side = 0; side < 2; side++)
	{
		const auto structure = pawn_structure(b, chess::SIDE(side));
		const int mult = side == b.side_on_move() ? 1 : -1;
		mat_count += mult * 75 * structure.first; // passed
		mat_count -= mult * 15 * structure.second; // doubled

	}

	// bishop colours - light/dark square strategy

	// central control

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

	if (b.halfmove_clock() >= 100)
	{
		return -contempt; // 50move draw

	}

	return nonterminal_value();

}


CHeuristic::~CHeuristic(void)
{
}
