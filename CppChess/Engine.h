#include "Board.h"
#include "UciSession.h"

class CEngine
{
public:
	CEngine(CUciSession & s, CBoard b);
	CMove Think();	
	~CEngine(void);

	typedef std::pair<int, CMove> MoveResult;
private:
	MoveResult negamax_root();
	
	void write_current_move(std::string, int);
	void write_best_move(std::string, int);

	CUciSession & _s;
	CBoard _b;
};
