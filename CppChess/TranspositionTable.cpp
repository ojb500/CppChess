#include "stdafx.h"
#include "TranspositionTable.h"

CTranspositionTable::CTranspositionTable(void)
	: _table(MAX_SIZE)
	//, _pv(MAX_SIZE)
	, occupancy(0)
	, collisions(0)
{
}

size_t CTranspositionTable::ZobristHasher::operator()(const CZobrist& zob)
{
	const size_t idx = zob.Hash() % CTranspositionTable::MAX_SIZE;
	ASSERT(idx < CTranspositionTable::MAX_SIZE);
	return idx;
}

namespace 
{
	size_t IndexFromZob(const CZobrist& zob)
	{
		return CTranspositionTable::ZobristHasher()(zob);
	}
}

boost::optional<STranspositionTableEntry> CTranspositionTable::get_entry(const CZobrist zob)const
{
	const STranspositionTableEntry& tte = _table[IndexFromZob(zob)];

	if (tte.depth > -1 && tte.zob == zob)
		return tte;
	return boost::none;
}

boost::optional<STranspositionTableEntry> CTranspositionTable::get_pv_entry(const CZobrist zob)const
{
	const auto& tte = _pvmap.find(zob);
	if (tte != _pvmap.end() && tte->second.depth > -1)
		return tte->second;
	return boost::none;
}

void CTranspositionTable::store_pv_entry(STranspositionTableEntry tte)
{
	_pvmap.insert(std::make_pair(tte.zob, tte));
}

void CTranspositionTable::store_entry(STranspositionTableEntry tte)
{
	const auto idx = IndexFromZob(tte.zob);

	if (tte.nt == NT_Exact)
	{
		store_pv_entry(tte);
	}

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
