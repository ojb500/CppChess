#include "stdafx.h"
#include "Engine.h"
#include <boost/optional.hpp>
#include "Perft.h"
#include "BoardMutator.h"
#include "Heuristic.h"

using namespace std;

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

	std::string uci_format_score(int score)
	{
		std::stringstream ss;
		if (score >= 9900 || score <= -9900)
		{
			ss << "mate ";
			int in_ply = 10000 - abs(score);
			int mating_or_mated = (score > 0 ? 1 : -1);
			ss << (1 + (in_ply / 2)) * mating_or_mated;
		}
		else
		{
			ss << "cp ";
			ss << score;
		}
		return ss.str();
	}
};

CEngine::CEngine(CUciSession & us)
	: _cancelled(false)
	, _s(us)
	, _nodes(0)
{
}

void CEngine::set_position(CBoard& b)
{
	_b = b;
}

namespace
{
	int see(CBoard& b, chess::SQUARES square, chess::SIDE side)
	{
		int value = 0;

		CBoard::INT_SQUARES smallestAttacker = b.get_smallest_attacker(side, CBoard::int_index(square));

		if (! CBoard::off_board(smallestAttacker))
		{
			const chess::PIECE piece_just_captured = b.piece_at_square(square).piece();
			CMove mv(CBoard::index(smallestAttacker), square, MOVE_CAPTURE);

			{
				CBoardMutator mut(b, mv);
				/* Do not consider captures if they lose material, therefore max zero */
				value = std::max(0, CHeuristic::piece_value(piece_just_captured) - see(b, square, other_side(side)) );
			}
		}
		return value;
	}

	int see_capture(CBoard& b, const CMove& m)
	{
		int value = 0;
		const chess::PIECE piece = b.piece_at_square(m.from()).piece();

		{
			CBoardMutator mut(b, m);
			value = CHeuristic::piece_value(piece) - see(b, m.to(), b.side_on_move());
		}
		return value;
	}

	int move_score(CBoard& b, const CMove& m, int ply, const boost::optional<STranspositionTableEntry>& tte)
	{
		// hash move
		if (tte)
		{
			if (tte->mv == m)
				return 10000;
		}

		int s=0;
		if (m.is_double_push())
			s+=5;
		if (m.is_castle())
			s+=10;
		if (m.is_capture())
		{
			const int static_exch_eval = see_capture(b, m);
			if (static_exch_eval > 0)
				s+=20 + static_exch_eval;
		}
		if (m.is_check())
			s+=10;
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

int CEngine::quiescence_negamax(int alpha, int beta)
{
	_qnodes++;
	int stand_pat_score = CHeuristic(_b).nonterminal_value();
	if (stand_pat_score >= beta)
		return beta;
	if (alpha < stand_pat_score)
		alpha = stand_pat_score;

	{
		int score;
		for (auto & mv : _b.legal_moves_q())
		{
			if (mv.is_capture() && !see_capture(_b, mv))
				continue;

			{
				CBoardMutator mut(_b, mv);
				score = -quiescence_negamax(-beta, -alpha);
			}

			if (score >= beta)
				return beta;
			if (score > alpha)
				alpha = score;
		}
	}
	return alpha;
}

int CEngine::negamax(int depth, int alpha, int beta, int color)
{
	_nodes++;

	boost::optional<STranspositionTableEntry> tte = tt.get_entry(_b.hash());

	if (tte)
	{
		if (tte->depth >= depth)
		{
			_hashhits++;
			if(tte->nt == NT_Exact) // stored value is exact
				return tte->value;
			if(tte->nt == NT_LB && tte->value > alpha) 
				alpha = tte->value; // update lowerbound alpha if needed
			else if(tte->nt == NT_UB && tte->value < beta)
				beta = tte->value; // update upperbound beta if needed
			if(alpha >= beta)
				return tte->value; // if lowerbound surpasses upperbound
		}
	}

	if (depth <= 0)
	{
		return quiescence_negamax(alpha, beta);
	}

	vector<pair<CMove, int>> moves_and_priorities;
	for (const auto & mv : _b.legal_moves())
	{
		moves_and_priorities.push_back(make_pair(mv, move_score(_b, mv, _b.ply(), tte)));
	}

	if (moves_and_priorities.size() == 0)
	{
		const int heur = CHeuristic(_b).value();
	}

	std::sort(moves_and_priorities.begin(), moves_and_priorities.end(), [&](const pair<CMove, int> & m1, const pair<CMove, int> & m2)
	{
		return m1.second > m2.second;
	});

	int n = 1;
	int best = -10000;
	int best_move_index = -1;
	int i = -1;
	for (const auto & mvp : moves_and_priorities)
	{
		i++;
		const auto & mv = mvp.first;
		{
			CBoardMutator mut(_b,mv);
			const int val = -TrimEvaluationForMate(negamax(depth - 1, -beta, -alpha, -color));
			if (val >= best)
			{
				best = val;
				best_move_index = i;
			}
			if (best > alpha)
			{
				alpha = best;
			}
			if (best >= beta)
				break;


		}
	}
	{
		NodeType nt;
		if (best <= alpha)
		{
			nt = NT_LB;
		}
		else if (best >= beta)
		{
			nt = NT_UB;
		}
		else
		{
			nt = NT_Exact;
		}
		tt.store_entry(STranspositionTableEntry(_b.hash(), depth, (best_move_index > -1 ? moves_and_priorities[best_move_index].first : CMove()), best, nt));
	}
	return best;
}

CEngine::MoveResult CEngine::negamax_root(int depth)
{
	auto tte = tt.get_entry(CZobrist(_b.hash()));

	int alpha = -10001;
	int beta = 10001;

	if (tte)
	{
		// blah
	}

	vector<pair<CMove, int>> moves_and_priorities;
	for (const auto & mv : _b.legal_moves())
	{
		moves_and_priorities.push_back(make_pair(mv, move_score(_b, mv, _b.ply(), tte)));
	}

	ASSERT(moves_and_priorities.size());

	const int color = 1;


	int best = -10001;
	boost::optional<CMove> best_move;

	const int ply = _b.ply();

	std::sort(moves_and_priorities.begin(), moves_and_priorities.end(), [&](const pair<CMove, int> & m1, const pair<CMove, int> & m2)
	{
		return m1.second > m2.second;
	});

	{
		std::stringstream ss;
		ss << "Legal moves (" << moves_and_priorities.size() << "): ";
		for (const auto & mv : moves_and_priorities)
			ss << mv.first.longer_algebraic() << " ";
		_s.WriteLogLine(ss.str());
	}

	int n = 1;
	for (const auto & mvp : moves_and_priorities)
	{
		const auto & mv = mvp.first;
		write_current_move(mv.long_algebraic(), n++);
		{
			CBoardMutator mut(_b,mv);
			const int val = -TrimEvaluationForMate(negamax(depth - 1, -beta, -alpha, -color));

			if (val >= beta)
				continue;
			if (val > alpha)
			{
				write_best_move(mv.long_algebraic(), val);
				best = val;
				best_move = mv;
				alpha = val;
			}
		}
	}

	{
		NodeType nt;
		if (best <= alpha)
		{
			nt = NT_LB;
		}
		else if (best >= beta)
		{
			nt = NT_UB;
		}
		else
		{
			nt = NT_Exact;
		}
		tt.store_entry(STranspositionTableEntry(_b.hash(), depth, *best_move, best, nt));
	}

//	output_pv();

	return std::make_pair(alpha, *best_move);
}

namespace {
	void get_pv(vector<CMove>& pv, CBoard& b, CTranspositionTable& tt)
	{
		if (pv.size() >= 20)
			return;

		auto maybeTte = tt.get_entry(b.hash());
		if (maybeTte)
		{
			auto tte = *maybeTte;
			if (tte.mv.to() != tte.mv.from() && tte.nt == NT_Exact)
			{
				auto tte = *maybeTte;
				pv.push_back(tte.mv);
				CBoardMutator mut(b, tte.mv);
				get_pv(pv, b, tt);
			}
		}
	}
}
void CEngine::output_pv()
{
	std::stringstream ss;
	ss << "info pv ";
	for (const auto & move : pv())
	{
		ss << move.long_algebraic() << " ";
	}
	_s.WriteLine(ss.str());
}
std::vector<CMove> CEngine::pv()
{
	vector<CMove> vpv;
	get_pv(vpv, _b, tt);
	return vpv;
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
	_s.WriteLogLine("Thinking\n" + _b.fen() + "\n" + _b.board());
	_nodes = 0;
	_qnodes = 0;





	MoveResult mr;
	std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now() ;
	mr = negamax_root(6);
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now() ;


	millisecs_t duration( std::chrono::duration_cast<millisecs_t>(end-start) ) ;

	std::stringstream ss;
	ss << (_nodes / (duration.count())) * 1000 << " nps, " << duration.count() << " msec";
	ss << _nodes << " nodes, " << _qnodes << " qnodes";
	_s.WriteLogLine(ss.str());

	return mr.second;
}

CMove CEngine::IterativeDeepening(millisecs_t timePerMove)
{
	{
		std::stringstream ss;
		ss << "Using iterative deepening with " << timePerMove.count() << "ms thinking time\n";
		ss << _b.fen() << "\n" << _b.board();
		_s.WriteLogLine(ss.str());
	}

	_nodes = 0;
	_qnodes = 0;

	MoveResult mr;
	std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now() ;
	for (int depth = 2; ; ++depth)
	{
		mr = negamax_root(depth);
		std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now() ;
		millisecs_t duration( std::chrono::duration_cast<millisecs_t>(now-start) ) ;
		std::stringstream ss;
		ss	<< "info score " << uci_format_score(mr.first) 
			<< " depth " << depth 
			<< " time " << duration.count() 
			<< " nodes " << _nodes 
			<< " nps " << int(_nodes / (duration.count() / 1000.0)) 
			<< " hashfull " << tt.permill_full() 
			<< " pv ";
		for (const auto mv : pv())
		{
			ss << mv.long_algebraic() << " ";
		}
		_s.WriteLine(ss.str());
		if ( _cancelled || duration > timePerMove )
			break;
	}


	std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now() ;
	millisecs_t duration( std::chrono::duration_cast<millisecs_t>(now-start) ) ;

	{
		std::stringstream ss;
		ss << int(_nodes / (duration.count() / 1000.0)) << " nps, " << duration.count() << " msec";
		ss << _nodes << " nodes, " << _qnodes << " qnodes";
		_s.WriteLogLine(ss.str());
	}

	return mr.second;
}

CEngine::~CEngine(void)
{
}
