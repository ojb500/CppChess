#include "stdafx.h"
#include "BoardMutator.h"

CBoardMutator::CBoardMutator(CBoard& b, const CMove mv)
	: _b(b)
	, _mv(mv)
#ifdef MUTATOR_CHECK_FENS
	, fen(b.fen())
#endif
#ifdef MUTATOR_CHECK_HASHES
	, zob(b.hash())
#endif
	, mem(b.make_move(mv))

{
};

CBoardMutator::~CBoardMutator()
{
	_b.unmake_move(mem);
#ifdef MUTATOR_CHECK_FENS
	ASSERT(fen == _b.fen());
#endif
#ifdef MUTATOR_CHECK_PIECELIST
	ASSERT(_b.assert_piecelist_consistent());
#endif
#ifdef MUTATOR_CHECK_HASHES
	ASSERT(_b.hash() == zob);
#endif
}