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
private:
	MoveResult negamax_root();
	
	void write_current_move(std::string, int);
	void write_best_move(std::string, int);

	int _nodes;

	CUciSession & _s;
	CBoard _b;
};
