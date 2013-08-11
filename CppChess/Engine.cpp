#include "stdafx.h"
#include "Engine.h"
#include <boost/optional.hpp>

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
class CBoardMutator
{
private:
	CBoard& _b;
	CMove _mv;
	CBoard::CMemento mem;
#ifdef _DEBUG
	std::string fen;
#endif
public:
	CBoardMutator(CBoard& b, const CMove mv)
		: _b(b)
		, _mv(mv)
		, mem(mv)
	{
#ifdef _DEBUG
		fen = b.fen();
#endif
		mem = b.make_move(mv);
	};

	~CBoardMutator()
	{
		_b.unmake_move(mem);
#ifdef _DEBUG
		ASSERT(fen == _b.fen());
#endif
	}
};

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
		s+=1;
	if (m.is_capture())
		s+=3;
	if (m.is_check())
		s+=3;
	if (m.is_promotion())
		s+=3;
	const int tofile = m.to() & 7;
	const int torank = m.to() >> 3;
	if ((torank == 3 || torank == 4) && (tofile == 3 || tofile == 4))
	{
		s+=1;
	}

	return s;
}


/*s*/ int CEngine::heuristic(CBoard& b)
{
	int color = (b.side_on_move() == chess::WHITE ? 1 : -1);
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
		{
			mat_count += 25 * mult;
		}
		// central pawn bonus
		else if (pc.first.piece() == chess::PAWN)
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
		else if (pc.first.piece() == chess::KNIGHT)
		{
			for (const auto & knight : pc.second)
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

	// central control todo

	// mobility
	int mobility = lm.size();

	//TODO
	return (mobility / 2) + mat_count;

}
namespace {
	struct PerftResult
	{
		unsigned long nodes;
		unsigned long ep;
		unsigned long captures;
		unsigned long promotions;
		unsigned long checks;
		unsigned long castles;

		PerftResult() : nodes(0), ep(0), captures(0), promotions(0), checks(0), castles(0) {}
		PerftResult& operator+= (const PerftResult & other)
		{
			nodes += other.nodes;
			/*ep += other.ep;
			captures += other.captures;
			promotions += other.promotions;
			checks += other.checks;
			*/
			return *this;
		};
	};

	PerftResult perft(CBoard & b, int depth)
	{
		PerftResult res;
		const auto & moves = b.legal_moves();
		if (depth == 1)
		{
			res.nodes = moves.size();
			return res;
		}
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

int CEngine::negamax(int depth, int alpha, int beta, int color)
{
	if (depth == 0)
		return color * heuristic(_b);

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
		return color * heuristic(_b);
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
			const int val = -negamax(depth - 1, -beta, -alpha, -color);
			if (val >= beta)
			{
				STranspositionTableEntry tte;
				tte.depth = depth - 1;
				tte.mv = mv;
				tte.nt = NT_LB;
				tte.score = beta;
				tt.store_entry(CZobrist(_b.hash()), tte);
				return val;
			}
			if (val > alpha)
			{
				STranspositionTableEntry tte;
				tte.depth = depth - 1;
				tte.mv = mv;
				tte.nt = NT_UB;
				tte.score = alpha;
				tt.store_entry(CZobrist(_b.hash()), tte);
				_pv[_b.ply() - 1] = mv;
				alpha = val;
			}
		}
	}
	
	return alpha;
}

CEngine::MoveResult CEngine::negamax_root()
{
#ifdef _DEBUG
	const int depth = 4;
#else
	const int depth = 5;
#endif

	auto tte = tt.get_entry(CZobrist(_b.hash()));
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
	int alpha = std::numeric_limits<int>::min();
	int beta = std::numeric_limits<int>::max();
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
			ss << mv.long_algebraic() << " ";
		_s.WriteLogLine(ss.str());
	}

	int n = 1;
	for (const auto & mv : lm)
	{
		write_current_move(mv.long_algebraic(), n++);
		{
			CBoardMutator mut(_b,mv);
			const int val = -negamax(depth - 1, -beta, -alpha, -color);

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
			if (i->to() == chess::SQUARE_LAST && i->from() == chess::SQUARE_LAST)
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

	auto result = negamax_root();
	return result.second;
}


CEngine::~CEngine(void)
{
}
