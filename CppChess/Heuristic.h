#pragma once
#include "Board.h"
#include "HeuristicParameters.h"

class CHeuristic
{
public:
	CHeuristic(CBoard& b);
	
	int value();
	int nonterminal_value();
	static int piece_value(chess::PIECE piece);

	~CHeuristic(void);

private:
	CBoard& b;
	static CHeuristicParameters params;
};

