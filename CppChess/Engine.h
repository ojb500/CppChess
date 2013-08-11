#include "Board.h"
#include "UciSession.h"
#include "TranspositionTable.h"

class CEngine
{
public:
	CEngine(CUciSession & s);
	CMove Think();	
	void Perft(int depth);
	~CEngine(void);

	void set_position(CBoard& b);

	typedef std::pair<int, CMove> MoveResult;

	static int heuristic (CBoard& b, chess::SIDE sideFor);

private:
	MoveResult negamax_root(int depth);
	int negamax(int depth, int alpha, int beta, int color);
	int quiescence_negamax(int alpha, int beta);

	typedef std::chrono::duration<int,std::milli> millisecs_t ;

	CTranspositionTable tt;

	int move_score(const CMove& m, int ply, const boost::optional<STranspositionTableEntry> & tte);

	void write_current_move(std::string, int);
	void write_best_move(std::string, int);

	

	int _nodes;

	std::vector<CMove> _pv;
	
	CUciSession & _s;
	CBoard _b;
};
