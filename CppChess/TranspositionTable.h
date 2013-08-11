#pragma once
#include <boost/optional.hpp>
#include "Zobrist.h"
#include "Move.h"

enum NodeType
{
	NT_Exact,
	NT_LB,
	NT_UB,
};

struct STranspositionTableEntry
{
	int depth;
	CMove mv;
	int score;
	NodeType nt;
};

class CTranspositionTable
{
public:
	CTranspositionTable(void);
	~CTranspositionTable(void);

	boost::optional<STranspositionTableEntry> get_entry(CZobrist zob);
	void store_entry(CZobrist zob, STranspositionTableEntry tte);

private:
	std::map<uint64_t, STranspositionTableEntry> _map;
};

