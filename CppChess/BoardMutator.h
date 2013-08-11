#pragma once
#include "Move.h"
#include "Board.h"
#include "Config.h"
#include "Zobrist.h"

class CBoardMutator
{
private:
	CBoard& _b;
	CMove _mv;
	#ifdef MUTATOR_CHECK_FENS
	std::string fen;
#endif
#ifdef MUTATOR_CHECK_HASHES
	CZobrist zob;
#endif

	CBoard::CMemento mem;
public:
	CBoardMutator(CBoard& b, const CMove mv);

	~CBoardMutator();
};