#include "Board.h"
#include "UciSession.h"
#include "TranspositionTable.h"

class CEngine
{
public:
	typedef std::chrono::duration<int,std::milli> millisecs_t ;

	CEngine(CUciSession & s, CBoard b);

	CEngine(CUciSession & s, CBoard b, millisecs_t wtime, millisecs_t winc, millisecs_t btime, millisecs_t binc, int movestogo, int maxdepth, int maxnodes);
	
	CMove Think();

	CMove iterative_deepening();


	void Perft(int depth);
	~CEngine(void);

	typedef std::pair<int, CMove> MoveResult;

	void cancel();

	void output_pv();
	std::vector<CMove> pv();

	bool should_cancel();


	class CExceptionCancelled : public std::exception
	{

	};

	class CKillerMoves
	{
	public:
		CKillerMoves();
		void add_killer(const int ply, const CMove mv);
		bool is_killer(const int ply, const CMove mv) const;
	private:
		std::vector<std::vector<CMove>> _killers;
	};

private:
	volatile bool _cancelled;

	std::chrono::steady_clock::time_point thinking_started;

	size_t distance_from_root;

	MoveResult negamax_root(int depth);
	int negamax(int depth, int alpha, int beta, int color);
	int quiescence_negamax(int alpha, int beta);

	static CTranspositionTable tt;

	void write_current_move(std::string, int);
	void write_best_move(std::string, int);

	millisecs_t _wtime;
	millisecs_t _btime;
	millisecs_t _winc;
	millisecs_t _binc;
	int _movestogo;
	int _maxdepth;
	int _maxnodes;

	millisecs_t _timeForThisMove;

	uint64_t _nodes;
	uint64_t _qnodes;
	uint64_t _hashhits;
	
	CUciSession & _s;
	CBoard _b;

	CKillerMoves _killers;
};
