#pragma once
#include "Piece.h"
#include "Square.h"

enum MOVE_FLAGS
{
	MOVE_NONE		= 0,
	MOVE_DBL_PUSH   = 1,
	MOVE_OO			= 2,
	MOVE_OOO		= 3,
	MOVE_CAPTURE	= 4,
	MOVE_EN_PASSANT	= 5,
	MOVE_PROMOTE_N  = 8,
	MOVE_PROMOTE_B	= 9,
	MOVE_PROMOTE_R  = 10,
	MOVE_PROMOTE_Q  = 11,
	MOVE_PROMOTE_XN	= 12,
	MOVE_PROMOTE_XB	= 13,
	MOVE_PROMOTE_XR	= 14,
	MOVE_PROMOTE_XQ	= 15,
	MOVE_PROMOTION  = 8, //  a b c d e f g h	
};

inline MOVE_FLAGS operator|(MOVE_FLAGS a, MOVE_FLAGS b)
{return static_cast<MOVE_FLAGS>(static_cast<int>(a) | static_cast<int>(b));}

class CMove
{
public:

	CMove()
		: _move(0)
	{
	}

	CMove(chess::SQUARES from, chess::SQUARES to, MOVE_FLAGS flags, chess::PIECE piece)
		: _move(((flags & 0xf)<<12) | ((from & 0x3f)<<6) | (to & 0x3f) | ((piece - 2) << 12))
	{
		ASSERT(is_promotion());
		ASSERT(promotion_piece() == piece);
	};

	CMove(chess::SQUARES from, chess::SQUARES to, MOVE_FLAGS flags)
		: _move(((flags & 0xf)<<12) | ((from & 0x3f)<<6) | (to & 0x3f))
	{
	};

	chess::SQUARES from() const { return static_cast<chess::SQUARES>((_move >> 6) & 0x3f); }
	chess::SQUARES to() const { return static_cast<chess::SQUARES>(_move & 0x3f); }
	MOVE_FLAGS flags() const { return static_cast<MOVE_FLAGS>((_move >> 12) & 0x0f); }
	static CMove FromString(std::string s)
	{
		ASSERT(s.length() == 4);
		chess::SQUARES from = chess::square_from_string(s.substr(0, 2));
		chess::SQUARES to = chess::square_from_string(s.substr(2, 2));
		return CMove(from, to, MOVE_NONE);
	};

	std::string longer_algebraic() const
	{
		std::stringstream ss;
		ss << chess::SQUARE_STRINGS[from()];
		if (is_capture())
		{
			ss << "x";
		}
		else
		{
			ss << "-";
		}
		ss << chess::SQUARE_STRINGS[to()];
		if (is_promotion())
		{
			ASSERT(false);
			// TODO
		}
		if (is_check())
		{
			ss << "+";
		}
		return ss.str();
	};

	std::string long_algebraic() const
	{
		std::stringstream ss;
		ss << chess::SQUARE_STRINGS[from()];
		ss << chess::SQUARE_STRINGS[to()];
		if (is_promotion())
		{
			ASSERT(false);
			// TODO
		}
		return ss.str();
	}

	bool is_castle()const
	{
		return flags() == MOVE_OO || flags() == MOVE_OOO;
	}

	bool is_oo()const
	{
		return flags() == MOVE_OO;
	}

	bool is_ooo()const
	{
		return flags() == MOVE_OOO;
	}

	bool is_check()const
	{
		return false; // TODO
		//return (flags() & MOVE_CHECK) != 0;
	}
	bool is_capture()const
	{
		return (flags() & MOVE_CAPTURE) != 0;
	};
	bool is_normal_move()const
	{
		return (flags() & ~MOVE_CAPTURE) == 0;
	}
	bool is_promotion()const
	{
		return (flags() & MOVE_PROMOTION) != 0;
	};
	bool is_double_push()const
	{
		return flags() == MOVE_DBL_PUSH;
	};
	bool is_en_passant_capture()const
	{
		return flags() == MOVE_EN_PASSANT;
	};
	chess::PIECE promotion_piece()const { 
		return static_cast<chess::PIECE>((flags() & 3) + chess::KNIGHT);
	};

	bool CMove::operator==(const CMove& rhs)const
	{
		return _move == rhs._move;
	}

	
private:
	unsigned int _move;
};

