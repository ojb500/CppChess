#pragma once
#include <boost/random.hpp>
#include "Piece.h"
#include "Square.h"

class CZobristHashes
{
private:
	uint64_t _keys[781];
public:
	CZobristHashes()
	{
		boost::random::mt19937_64 mt;

		for (int i = 0; i < _countof(_keys); ++i)
		{
			_keys[i] = mt();
		}

	}
	const uint64_t operator[](const int& i)
	{
		return _keys[i];
	}
};

class CZobrist
{
public:
	enum FeatureHash
	{
		PIECE_ON_SQUARE_BEGIN = 0,
		SIDE_ON_MOVE_BEGIN = PIECE_ON_SQUARE_BEGIN + (12*64),
		CASTLING_RIGHTS_BEGIN = SIDE_ON_MOVE_BEGIN + 1,
		EN_PASSANT_FILE_BEGIN = CASTLING_RIGHTS_BEGIN + 4,
		HASH_LAST = EN_PASSANT_FILE_BEGIN + 8,
	};

private:
	static CZobristHashes HASHES;
	uint64_t _hashkey;

	void ApplyIndex(int hash);

public:
	explicit CZobrist(uint64_t);
	
	void ApplyPieceAtSquare(CPiece piece, chess::SQUARES square);
	void SwitchSideOnMove();
	void Apply(CZobrist other);

	void ApplyEnPassantFile(int file);

	uint64_t Hash()const;
};

