#include "Board.h"
#include "UciSession.h"

class CEngine
{
public:
	CEngine(CUciSession & s, CBoard b);
	CMove Think();	
	void Perft(int depth);
	~CEngine(void);

	typedef std::pair<int, CMove> MoveResult;

	static int heuristic (CBoard& b);

private:
	MoveResult negamax_root();
	int negamax(int depth, int alpha, int beta, int color);

	void write_current_move(std::string, int);
	void write_best_move(std::string, int);

	

	int _nodes;

	std::vector<CMove> _pv;
	
	CUciSession & _s;
	CBoard _b;
};
