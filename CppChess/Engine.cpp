#include "stdafx.h"
#include "Engine.h"
#include <boost/optional.hpp>
#include "Perft.h"
#include "BoardMutator.h"

namespace
{
	int TrimEvaluationForMate(int eval)
	{
		if (eval >= 9900)
		{
			return eval - 1;
		}
		else if (eval <= -9900)
		{
			return eval + 1;
		}
		return eval;
	}
};

CEngine::CEngine(CUciSession & us)
	: _s(us)
	, _nodes(0)
{
	_pv.resize(100);
}

void CEngine::set_position(CBoard& b)
{
	_b = b;
}

int CEngine::move_score(const CMove& m, int ply, const boost::optional<STranspositionTableEntry>& tte)
{
	// killer
	if (tte)
	{
		if (tte->mv == m)
			return 10000;
	}

	if (_pv[ply] == m)
		return 10000;

	int s=0;
	if (m.is_double_push())
		s+=5;
	if (m.is_capture())
	{
		s+=10;
	}
	if (m.is_check())
		s+=15;
	if (m.is_promotion())
		s+=10;
	const int tofile = m.to() & 7;
	const int torank = m.to() >> 3;
	if ((torank == 3 || torank == 4) && (tofile == 3 || tofile == 4))
	{
		s+=1;
	}

	return s;
}


/*s*/ int CEngine::heuristic(CBoard& b, chess::SIDE sideFor)
{
	int color = (sideFor == chess::WHITE ? 1 : -1);
	auto lm = b.legal_moves();

	if (!lm.size())
	{
		// checkmated or draw
		if (b.is_check())
		{
			// checkmated
			return sideFor == b.side_on_move() ? -10000 : 10000;
		}
		else
		{
			return 0; // drawn position
		}
	}

	// material count
	int mat_count = 0;
	const int values[] = {
		0, // NOTHING = 0,
		100, // PAWN = 1,
		300, // KNIGHT = 2,
		300, // BISHOP = 3,
		500, // ROOK = 4,
		900, // QUEEN = 5,
		100000, // KING = 6,
	};
	for (int side=0; side<2; side++)
	{
		for (int piece=1; piece<7; piece++)
		{
			const CPiece p(side == 0 ? chess::WHITE : chess::BLACK, chess::PIECE(piece));
			const auto & pc = b.piece_table()[p];

			int mult = (p.side() == sideFor ? 1 : -1);
			mat_count += pc.size() * values[p.piece()] * mult;
			// 2 bishop bonus
			if (p.piece() == chess::BISHOP && pc.size() > 1)
			{
				mat_count += 25 * mult;
			}
			// central pawn bonus
			else if (p.piece() == chess::PAWN)
			{
				for (const auto & pawn : pc)
				{ 
					const int file07 = pawn & 7;
					const int rank07 = pawn >> 4;
					if ((file07 == 3 || file07 == 4)
						&& (rank07 == 3 || rank07 == 4))
						mat_count += 10 * mult;
				}
			}
			else if (p.piece() == chess::KNIGHT)
			{
				for (const auto & knight : pc)
				{ 
					const int file07 = knight & 7;
					const int rank07 = knight >> 4;
					if ((file07 > 1 && file07 < 6)
						&& (rank07 > 1 && rank07 < 6))
						mat_count += 10 * mult;
					if (file07 == 0 || rank07 == 0 || file07 == 7 || rank07 == 7)
						mat_count -= 10 * mult;
				}
			}
		}
	}

	// central control todo

	// mobility
	int mobility = lm.size();

	//TODO
	return (mobility / 2) + mat_count;

}

void CEngine::Perft(int maxdepth)
{
	_s.WriteLine(_b.board());
	for (int i = 1; i <= maxdepth; i++)
	{
		{
			std::stringstream ss;
			ss << "perft(" << i << ") ";
			_s.WriteLine(ss.str());
		}
		{
			CPerft perft(_b);
			std::stringstream ss;
			std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now() ;
			auto nodes = perft.DoPerft(i);
			std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now() ;

			typedef std::chrono::duration<int,std::milli> millisecs_t ;

			millisecs_t duration( std::chrono::duration_cast<millisecs_t>(end-start) ) ;
			ss << "  " << "= " << nodes.nodes << ", " << duration.count() << " msec" << std::endl;
			ss << "    caps " << nodes.captures << ", eps " << nodes.ep << ", oo " << nodes.castles << ", proms " << nodes.promotions << ", checks " << nodes.checks;
			_s.WriteLine(ss.str());
		}
	}	
	_s.WriteLine("done");
}

//int CEngine::quiescence_negamax(int depth, int alpha, int beta, int color)
//{
//}

int CEngine::negamax(int depth, int alpha, int beta, int color)
{
	_nodes++;
	if (depth == 0)
		return heuristic(_b, _b.side_on_move());

	auto tte = tt.get_entry(CZobrist(_b.hash()));

	if (tte)
	{
		if (tte->depth >= depth)
		{
			return tte->score;
		}
	}

	auto lm = _b.legal_moves();
	if (lm.size() == 0)
	{
		return heuristic(_b, _b.side_on_move());
	}
	const int ply = _b.ply();
	std::sort(lm.begin(), lm.end(), [&](const CMove & m1, const CMove & m2)
	{
		return move_score(m1, ply, tte) > move_score(m2, ply, tte);
	});

	for (const auto & mv : lm)
	{
		{
			CBoardMutator mut(_b,mv);
			const int val = -TrimEvaluationForMate(negamax(depth - 1, -beta, -alpha, -color));
			if (val >= beta)
			{
				return val;
			}
			if (val > alpha)
			{
				_pv[_b.ply() - 1] = mv;
				alpha = val;
			}
		}
	}

	return alpha;
}

CEngine::MoveResult CEngine::negamax_root(int depth)
{
	auto tte = tt.get_entry(CZobrist(_b.hash()));

	int alpha = std::numeric_limits<int>::min();
	int beta = std::numeric_limits<int>::max();

	if (tte)
	{
		if (tte->depth >= depth)
		{
			MoveResult mr;
			mr.first = tte->score;
			mr.second = tte->mv;
			return mr;
		}
	}

	auto lm = _b.legal_moves();

	if (lm.size() == 0)
	{
		ASSERT(false);
	}
	const int color = 1;


	boost::optional<CMove> best;

	const int ply = _b.ply();

	std::sort(lm.begin(), lm.end(), [&](const CMove & m1, const CMove & m2)
	{
		return move_score(m1, ply, tte) > move_score(m2, ply, tte);
	});

	{
		std::stringstream ss;
		ss << "Legal moves (" << lm.size() << "): ";
		for (const auto & mv : lm)
			ss << mv.longer_algebraic() << " ";
		_s.WriteLogLine(ss.str());
	}

	int n = 1;
	for (const auto & mv : lm)
	{
		write_current_move(mv.long_algebraic(), n++);
		{
			CBoardMutator mut(_b,mv);
			const int val = -TrimEvaluationForMate(negamax(depth - 1, -beta, -alpha, -color));

			if (val >= beta)
				continue;
			if (val > alpha)
			{
				write_best_move(mv.long_algebraic(), val);
				_pv[_b.ply() - 1] = mv;
				best = mv;
				alpha = val;
			}
		}
	}
	{
		std::stringstream ss;
		ss << "info pv ";
		for (auto i = _pv.begin() + _b.ply(); i != _pv.end(); ++i )
		{
			if (i->to() == chess::SQUARE_LAST &&  i->from() == chess::SQUARE_LAST)
				break;

			ss << i->long_algebraic();
			ss << " ";
		}
		_s.WriteLine(ss.str());
	}
	return std::make_pair(alpha, *best);
}

void CEngine::write_current_move(std::string s, int i)
{
	std::stringstream ss;
	ss << "info currmove " << s << " currmovenumber "<< i;
	_s.WriteLine(ss.str());
}

void CEngine::write_best_move(std::string s, int i)
{
	std::stringstream ss;
	ss << "info bestmove " << s << " score cp "<< i;
	_s.WriteLine(ss.str());
}

CMove CEngine::Think()
{
	_s.WriteLogLine("Thinking\n" + _b.board());
	_nodes = 0;





	MoveResult mr;
	std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now() ;
	mr = negamax_root(6);
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now() ;


	millisecs_t duration( std::chrono::duration_cast<millisecs_t>(end-start) ) ;

	std::stringstream ss;
	ss << (_nodes / (duration.count())) * 1000 << " nps, " << duration.count() << " msec";
	_s.WriteLogLine(ss.str());

	return mr.second;
}


CEngine::~CEngine(void)
{
}
