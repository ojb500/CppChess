#pragma once
#include "Piece.h"
#include "Square.h"

enum MOVE_FLAGS
{
	MOVE_NONE		= 0,
	MOVE_OO			= 1,
	MOVE_OOO		= 2,
	MOVE_CHECK		= 4,
	MOVE_CAPTURE	= 8,
	MOVE_PROMOTION  = 16, //  a b c d e f g h	
	MOVE_EN_PASSANT	= 32,
	MOVE_DBL_PUSH   = 64,
};

inline MOVE_FLAGS operator|(MOVE_FLAGS a, MOVE_FLAGS b)
{return static_cast<MOVE_FLAGS>(static_cast<int>(a) | static_cast<int>(b));}

class CMove
{
public:

	CMove()
		: _from(chess::SQUARE_LAST)
		, _to(chess::SQUARE_LAST)
		, _flags(MOVE_NONE)
		, _promotion(chess::NOTHING)
	{
	}

	CMove(chess::SQUARES from, chess::SQUARES to, MOVE_FLAGS flags)
		: _from(from)
		, _to(to)
		, _flags(flags)
		, _promotion(chess::NOTHING)
	{
		ASSERT(!(flags & MOVE_PROMOTION));
	};

	CMove(chess::SQUARES from, chess::SQUARES to, MOVE_FLAGS flags, chess::PIECE promotion)
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

	bool is_oo()const
	{
		return (_flags & MOVE_OO) != 0;
	}

	bool is_ooo()const
	{
		return (_flags & MOVE_OOO) != 0;
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
	bool is_double_push()const
	{
		return (_flags & MOVE_DBL_PUSH) != 0;
	};
	bool is_en_passant_capture()const
	{
		return (_flags & MOVE_EN_PASSANT) != 0;
	};
	chess::PIECE promotion_piece()const { 
		return _promotion;
	};

	bool CMove::operator==(const CMove& rhs)const
	{
		return (_to == rhs._to && _from == rhs._from && _flags == rhs._flags && _promotion == rhs._promotion);
	}
	
private:
	chess::SQUARES _from;
	chess::SQUARES _to;
	MOVE_FLAGS _flags;
	chess::PIECE _promotion;
};

