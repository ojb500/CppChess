#include "stdafx.h"
#include "Zobrist.h"

CZobristHashes CZobrist::HASHES;

CZobrist::CZobrist(uint64_t k)
	: _hashkey(k)
{
}

void CZobrist::ApplyIndex(int index)
{
	ASSERT(index >= 0 && index <= HASH_LAST);
	_hashkey ^= HASHES[index];
}

void CZobrist::ApplyPieceAtSquare(CPiece piece, chess::SQUARES square)
{
	ASSERT(piece && piece.piece());
	const int p = piece - 1;
	const int index = PIECE_ON_SQUARE_BEGIN + p + (square*12);
	ASSERT(index <= 800);
	ApplyIndex(index);
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

CZobrist::operator uint64_t() const
{
	return _hashkey;
}