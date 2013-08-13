#pragma once
#include "Piece.h"
#include "Square.h"

// Soon this will read all the parameters from a file so that we can have a fight.
class CHeuristicParameters
{
public:
	CHeuristicParameters(void);
	~CHeuristicParameters(void);

	int GetValue(CPiece piece, chess::SQUARES sq);
private:
	std::vector<int> _base_values;
	std::vector<std::vector<int>> _piece_squares;
};
