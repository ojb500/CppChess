#include "stdafx.h"
#include "BoardMutator.h"

// Config.h contains the #defines here which turn on/off extra integrity checks

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
#ifdef MUTATOR_CHECK_HASHES
	RELEASE_ASSERT(b.hash() != zob);
#endif
};

CBoardMutator::~CBoardMutator()
{
	_b.unmake_move(mem);
#ifdef MUTATOR_CHECK_FENS
	RELEASE_ASSERT(fen == _b.fen());
#endif
#ifdef MUTATOR_CHECK_PIECELIST
	RELEASE_ASSERT(_b.assert_piecelist_consistent());
#endif
#ifdef MUTATOR_CHECK_HASHES
	RELEASE_ASSERT(_b.hash() == zob);
#endif
}