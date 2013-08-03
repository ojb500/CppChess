#include "stdafx.h"
#include "Engine.h"
#include <boost/optional.hpp>

CEngine::CEngine(CUciSession & us, CBoard b)
	: _b(b)
	, _s(us)
	, _nodes(0)
{
}

class CBoardMutator
{
private:
	CBoard& _b;
	CMove _mv;
	CBoard::CMemento mem;
public:
	CBoardMutator(CBoard& b, const CMove mv)
		: _b(b)
		, _mv(mv)
		, mem(mv)
	{
		mem = b.make_move(mv);
	};

	~CBoardMutator()
	{
		_b.unmake_move(mem);
	}
};

namespace
{
	int heuristic(CBoard b)
	{
		int color = (b.side_on_move() == chess::WHITE ? 1 : -1);
		auto lm = b.legal_moves();

		if (!lm.size())
		{
			// checkmated or draw
			// todo
			if (b.is_check())
			{
				// checkmated
				return -10000;
			}
			else
			{
				return 0; // drawn position
			}
			ASSERT(false);
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
		for (const auto & pc : b.piece_table())
		{
			int mult = (pc.first.side() == b.side_on_move() ? 1 : -1);
			mat_count += pc.second.size() * values[pc.first.piece()] * mult;
			// 2 bishop bonus
			if (pc.first.piece() == chess::BISHOP && pc.second.size() > 1)
				mat_count += 25 * mult;

			// central pawn bonus
			if (pc.first.piece() == chess::PAWN)
			{
				for (const auto & pawn : pc.second)
				{ 
					const int file07 = pawn & 7;
					const int rank07 = pawn >> 4;
					if ((file07 == 3 || file07 == 4)
						&& (rank07 == 3 || rank07 == 4))
						mat_count += 10 * mult;
				}
			}
		}

		// central control todo

		// mobility
		int mobility = lm.size();

		//TODO
		return (mobility / 2) + mat_count;
	}

	int negamax(CBoard b, int depth, int alpha, int beta, int color)
	{
		if (depth == 0)
			return color * heuristic(b);
		const auto lm = b.legal_moves();
		if (lm.size() == 0)
		{
			return color * heuristic(b);
		}
		for (const auto & mv : lm)
		{
			CBoardMutator mut(b,mv);
			const int val = -negamax(b, depth - 1, -beta, -alpha, -color);
			if (val >= beta)
				return val;
			if (val > alpha)
				alpha = val;
		}
		return alpha;
	}
};

namespace {
	struct PerftResult
	{
		unsigned long nodes;
		unsigned long ep;
		unsigned long captures;
		unsigned long promotions;
		unsigned long checks;
		unsigned long castles;

		PerftResult() : nodes(1), ep(0), captures(0), promotions(0), checks(0), castles(0) {}
		PerftResult& operator+= (const PerftResult & other)
		{
			nodes += other.nodes;
			ep += other.ep;
			captures += other.captures;
			promotions += other.promotions;
			checks += other.checks;
			return *this;
		};
	};

	PerftResult perft(CBoard & b, int depth)
	{
		PerftResult res;
		if (depth == 0) return res;
		res.nodes = 0;
		const auto & moves = b.legal_moves();
		for (const auto & move : moves)
		{
			{
				// test flags
				if (move.is_capture())
					res.captures++;
				if (move.is_check())
					res.checks++;
				if (move.is_promotion())
					res.promotions++;
				if (move.is_en_passant_capture())
					res.ep++;
				if (move.is_castle())
					res.castles++;
			}
			auto mem = b.make_move(move);
			res += perft(b, depth-1);
			b.unmake_move(mem);
		}
		return res;
	}
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
			std::stringstream ss;
			std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now() ;
			auto nodes = perft(_b, i);
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

CEngine::MoveResult CEngine::negamax_root()
{

	const auto lm = _b.legal_moves();
	if (lm.size() == 0)
	{
		ASSERT(false);
	}
	int alpha = std::numeric_limits<int>::min();
	int beta = std::numeric_limits<int>::max();
	const int color = 1;
#ifdef _DEBUG
	const int depth = 5;
#else
	const int depth = 6;
#endif


	boost::optional<CMove> best;

	int n = 1;
	for (const auto & mv : lm)
	{
		write_current_move(mv.long_algebraic(), n++); 
		CBoardMutator mut(_b,mv);
		const int val = -negamax(_b, depth - 1, -beta, -alpha, -color);
		if (val >= beta)
			continue;
		if (val > alpha)
		{
			write_best_move(mv.long_algebraic(), val);
			best = mv;
			alpha = val;
		}
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
	_nodes = 0;

	auto result = negamax_root();
	return result.second;
}


CEngine::~CEngine(void)
{
}
