#pragma once

namespace chess
{
	enum SIDE
	{
		WHITE = 0,
		BLACK = 1,
		SIDE_FIRST = WHITE,
		SIDE_LAST = BLACK,
	};

	enum PIECE
	{
		NOTHING = 0,
		PAWN = 1,
		KNIGHT = 2,
		BISHOP = 3,
		ROOK = 4,
		QUEEN = 5,
		KING = 6,
		PIECE_FIRST = PAWN,
		PIECE_LAST = KING,
	};
}

inline chess::SIDE other_side(chess::SIDE s)
{
	return static_cast<chess::SIDE>(!s);
}

inline chess::PIECE operator++(chess::PIECE p)
{return static_cast<chess::PIECE>(static_cast<int>(p) + 1);}

class CPiece
{
public:
	CPiece(chess::SIDE s, chess::PIECE p)
		: _b((s << 3) | p)
	{
		ASSERT(s >= chess::SIDE_FIRST && s <= chess::SIDE_LAST);
		ASSERT(p >= chess::NOTHING && p <= chess::PIECE_LAST);
	}

	CPiece()
		: _b(0)
	{
	}

	chess::SIDE side()const
	{
		return (chess::SIDE)(_b >> 3);
	}

	chess::PIECE piece()const
	{
		return (chess::PIECE)(_b & 7);
	}

	const std::string as_string()const
	{
		const std::string pieces[] = {"", "P", "N", "B", "R", "Q", "K"};

		return pieces[piece()];
	}

	const std::string as_side_string()const
	{
		const std::string white_pieces[] = {"", "P", "N", "B", "R", "Q", "K"};
		const std::string black_pieces[] = {"", "p", "n", "b", "r", "q", "k"};

		return side() == chess::WHITE ? white_pieces[piece()] : black_pieces[piece()];
	}

	operator unsigned short() const
	{
		return _b;
	};

	static CPiece from_char(char s)
	{
		const char pieces[] = { 'P', 'N', 'B', 'R', 'Q', 'K',
								'p', 'n', 'b', 'r', 'q', 'k'};
		for (int i=0; i<_countof(pieces); i++)
		{
			if (s == pieces[i])
			{
				return CPiece(i > 5 ? chess::BLACK : chess::WHITE, chess::PIECE(1 + (i % 6)));
			}

		}
		return CPiece();
	};

private:
	unsigned short _b;
};

struct less_piece
{
	bool operator()(const CPiece & p1, const CPiece & p2)
	{
		return p1 < p2;
	}
};
