#include "stdafx.h"
#include "TranspositionTable.h"

CTranspositionTable::CTranspositionTable(void)
	: _table(MAX_SIZE)
	, occupancy(0)
	, collisions(0)
{
}

namespace 
{
	size_t IndexFromZob(const CZobrist zob)
	{
		  const size_t idx = zob.Hash() % CTranspositionTable::MAX_SIZE;
		  ASSERT(idx < CTranspositionTable::MAX_SIZE);
		  return idx;
	}
}

boost::optional<STranspositionTableEntry> CTranspositionTable::get_entry(const CZobrist zob)const
{
	boost::optional<STranspositionTableEntry> tte = _table[IndexFromZob(zob)];
	if (tte->depth > -1 && tte->zob == zob)
		return tte;
	return boost::none;
}

void CTranspositionTable::store_entry(STranspositionTableEntry tte)
{
	const auto idx = IndexFromZob(tte.zob);
	if (_table[idx].depth == -1)
	{
		occupancy++;
	}
	else
	{
		if (tte.zob != _table[idx].zob)
		{
			collisions++;
		}
		else
		{
			// Only replace if depth greater
			if (tte.depth < _table[idx].depth)
				return;
		}
	}

	_table[idx] = tte;
}

int CTranspositionTable::permill_full()const
{
	return int((occupancy / static_cast<float>(MAX_SIZE)) * 1000);
}

CTranspositionTable::~CTranspositionTable(void)
{
}
