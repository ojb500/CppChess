#pragma once
#include "Piece.h"
#include "Square.h"

// Soon this will read all the parameters from a file so that we can have a fight.
class CHeuristicParameters
{
public:
	CHeuristicParameters(void);
	~CHeuristicParameters(void);

	int get_value(chess::PIECE piece);
	int get_value(CPiece piece, chess::SQUARES sq, int stage);
private:
	std::vector<int> _base_values;
	std::vector<std::vector<int>> _piece_squares_opening;
	std::vector<std::vector<int>> _piece_squares_endgame;
};

