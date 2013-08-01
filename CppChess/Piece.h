#pragma once

namespace chess
{
	enum SIDE
	{
		NONE = 0,
		WHITE = 16,
		BLACK = 32,
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

class CPiece
{
public:
	CPiece(chess::SIDE s, chess::PIECE p)
		: _b(s | p)
	{
	}

	CPiece()
		: _b(0)
	{
	}

	chess::SIDE side()const
	{
		return (chess::SIDE)(_b & (chess::WHITE | chess::BLACK));
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

	short as_short() const
	{
		return _b;
	};

private:
	short _b;
};

struct less_piece
{
	bool operator()(const CPiece & p1, const CPiece & p2)
	{
		return p1.as_short() < p2.as_short();
	}
};
