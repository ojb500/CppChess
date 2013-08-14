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
	int depth;
	CMove mv;
	int value;
	NodeType nt;
	
	STranspositionTableEntry()
		: depth(-1)
		, mv()
		, value(-9999)
		, nt(NT_LB)
	{}

	STranspositionTableEntry(int d, CMove mv, int val, NodeType nt)
		: depth(d)
		, mv(mv)
		, value(val)
		, nt(nt)
	{}

};

class CTranspositionTable
{
public:
	const static size_t MAX_SIZE = 1024000;

	CTranspositionTable(void);
	~CTranspositionTable(void);

	boost::optional<STranspositionTableEntry> get_entry(CZobrist zob)const;
	void store_entry(CZobrist zob, STranspositionTableEntry tte);

	int permill_full()const;

private:
	std::unordered_map<uint64_t, STranspositionTableEntry> _map;
};

