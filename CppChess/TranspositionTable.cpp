#include "stdafx.h"
#include "TranspositionTable.h"

CTranspositionTable::CTranspositionTable(void)
	: _table(MAX_SIZE)
{
}

namespace 
{
	int IndexFromZob(const CZobrist zob)
	{
		return zob.Hash() & 0xFFFFF;
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
	if (tte.depth > 0)
		_table[IndexFromZob(tte.zob)] = tte;
}

int CTranspositionTable::permill_full()const
{
	return 0;
}

CTranspositionTable::~CTranspositionTable(void)
{
}
