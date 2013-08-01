#pragma once
#include "Piece.h"
#include "Square.h"

class CMove
{
public:
	enum FLAGS
	{
		MOVE_NONE		= 0,
		MOVE_OO			= 1,
		MOVE_OOO		= 2,
		MOVE_CHECK		= 4,
		MOVE_CAPTURE	= 8,
		MOVE_PROMOTION  = 16, //  a b c d e f g h	
		MOVE_EN_PASSANT	= 32,
	};
	CMove(chess::SQUARES from, chess::SQUARES to, FLAGS flags)
		: _from(from)
		, _to(to)
		, _flags(flags)
		, _promotion()
	{
		ASSERT(!(flags & MOVE_PROMOTION));
	};

	CMove(chess::SQUARES from, chess::SQUARES to, FLAGS flags, CPiece promotion)
		: _from(from)
		, _to(to)
		, _flags(flags)
		, _promotion(promotion)
	{
		ASSERT(flags & MOVE_PROMOTION);
	};

	chess::SQUARES from() const { return _from; };
	chess::SQUARES to() const { return _to; };

	static CMove FromString(std::string s)
	{
		ASSERT(s.length() == 4);
		chess::SQUARES from = chess::square_from_string(s.substr(0, 2));
		chess::SQUARES to = chess::square_from_string(s.substr(2, 2));
		return CMove(from, to, MOVE_NONE);
	};

	std::string long_algebraic() const
	{
		std::stringstream ss;
		ss << chess::SQUARE_STRINGS[_from];
		ss << chess::SQUARE_STRINGS[_to];
		if (is_promotion())
		{
			ASSERT(false);
			// TODO
		}
		return ss.str();
	}

	bool is_castle()const
	{
		return (_flags & (MOVE_OO | MOVE_OOO)) != 0;
	}

	bool is_check()const
	{
		return (_flags & MOVE_CHECK) != 0;
	}
	bool is_capture()const
	{
		return (_flags & MOVE_CAPTURE) != 0;
	};
	bool is_promotion()const
	{
		return (_flags & MOVE_PROMOTION) != 0;
	};
	CPiece promotion_piece()const { 
		return _promotion;
	};

private:
	chess::SQUARES _from;
	chess::SQUARES _to;
	FLAGS _flags;
	CPiece _promotion;
};

struct CMemento
{
public:
	CMemento(CMove mv) : move(mv) {};

	CPiece captured;
	chess::CASTLING_RIGHTS cr;
	CMove move;
};