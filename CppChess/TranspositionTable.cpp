#include "stdafx.h"
#include "TranspositionTable.h"

CTranspositionTable::CTranspositionTable(void)
{
}

boost::optional<STranspositionTableEntry> CTranspositionTable::get_entry(CZobrist zob)
{
	auto mi = _map.find(zob.Hash());
	if (mi != _map.end())
		return mi->second;
	return boost::none;
}

void CTranspositionTable::store_entry(CZobrist zob, STranspositionTableEntry tte)
{
	if (tte.depth > 0)
		_map[zob.Hash()] = tte;
}

CTranspositionTable::~CTranspositionTable(void)
{
}
