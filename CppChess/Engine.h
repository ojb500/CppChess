#include "Board.h"
#include "UciSession.h"
#include "TranspositionTable.h"

class CEngine
{
public:
	typedef std::chrono::duration<int,std::milli> millisecs_t ;

	CEngine(CUciSession & s);
	
	CMove Think();

	CMove IterativeDeepening(millisecs_t timePerMove);


	void Perft(int depth);
	~CEngine(void);

	void set_position(CBoard& b);

	typedef std::pair<int, CMove> MoveResult;

	void cancel();

	void output_pv();
	std::vector<CMove> pv();

private:
	volatile bool _cancelled;

	MoveResult negamax_root(int depth);
	int negamax(int depth, int alpha, int beta, int color);
	int quiescence_negamax(int alpha, int beta);

	CTranspositionTable tt;

	void write_current_move(std::string, int);
	void write_best_move(std::string, int);

	

	uint64_t _nodes;
	uint64_t _qnodes;
	uint64_t _hashhits;
	
	CUciSession & _s;
	CBoard _b;
};
