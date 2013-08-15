#pragma once
#include <boost/optional.hpp>
#include <unordered_map>
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
	CZobrist zob;
	int depth;
	CMove mv;
	int value;
	NodeType nt;
	
	STranspositionTableEntry()
		: zob(0)
		, depth(-1)
		, mv()
		, value(-9999)
		, nt(NT_LB)
	{}

	STranspositionTableEntry(CZobrist zob, int d, CMove mv, int val, NodeType nt)
		: zob(zob)
		, depth(d)
		, mv(mv)
		, value(val)
		, nt(nt)
	{}

};

class CTranspositionTable
{
public:
	const static size_t MAX_SIZE = 0x100000;

	CTranspositionTable(void);
	~CTranspositionTable(void);

	boost::optional<STranspositionTableEntry> get_entry(const CZobrist zob)const;
	void store_entry(STranspositionTableEntry tte);

	int permill_full()const;

private:
	std::vector<STranspositionTableEntry> _table;
};

