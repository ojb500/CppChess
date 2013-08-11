#include "stdafx.h"
#include "Zobrist.h"

CZobristHashes CZobrist::HASHES;

CZobrist::CZobrist(uint64_t k)
	: _hashkey(k)
{
}

void CZobrist::ApplyIndex(int index)
{
	ASSERT(index <= HASH_LAST);
	_hashkey ^= HASHES[index];
}

void CZobrist::ApplyPieceAtSquare(CPiece piece, chess::SQUARES square)
{
	const int p = (piece.piece() - 1) + (piece.side() == chess::WHITE ? 6 : 0);
	ApplyIndex(PIECE_ON_SQUARE_BEGIN + (square << 4) | p);
}

void CZobrist::SwitchSideOnMove()
{
	ApplyIndex(SIDE_ON_MOVE_BEGIN);
}

void CZobrist::ApplyEnPassantFile(int file)
{
	ApplyIndex(EN_PASSANT_FILE_BEGIN + file);
}

uint64_t CZobrist::Hash()const
{
	return _hashkey;
}

void CZobrist::Apply(CZobrist other)
{
	_hashkey ^= other.Hash();
}