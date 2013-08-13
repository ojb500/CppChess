#pragma once
#include "Board.h"
#include "HeuristicParameters.h"

class CHeuristic
{
public:
	CHeuristic(CBoard& b);
	
	int value();

	~CHeuristic(void);

private:
	CBoard& b;
	static CHeuristicParameters params;
};

