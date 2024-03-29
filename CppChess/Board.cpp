#include "stdafx.h"
#include "Board.h"
#include "BoardMutator.h"
#include "HeuristicParameters.h"

namespace
{
	const CBoard::INT_SQUARES oo_rook_from_square[]		=	{ CBoard::H1, CBoard::H8 };
	const CBoard::INT_SQUARES oo_rook_to_square[]		=	{ CBoard::F1, CBoard::F8 };
	const CBoard::INT_SQUARES oo_king_from_square[]		=	{ CBoard::E1, CBoard::E8 };
	const CBoard::INT_SQUARES oo_king_to_square[]		=	{ CBoard::G1, CBoard::G8 };
	const CBoard::INT_SQUARES ooo_rook_from_square[]	=	{ CBoard::A1, CBoard::A8 };
	const CBoard::INT_SQUARES ooo_rook_to_square[]		=	{ CBoard::D1, CBoard::D8 };
	const CBoard::INT_SQUARES ooo_king_from_square[]	=	{ CBoard::E1, CBoard::E8 };
	const CBoard::INT_SQUARES ooo_king_to_square[]		=	{ CBoard::C1, CBoard::C8 };

};

const CBoard::PieceTable& CBoard::piece_table()const
{
	return _pieces;
};

CBoard::LookupTables CBoard::_lookups(CBoard::createLookupTables());

void CBoard::AddOffsetAttacks(CBoard::INT_SQUARES start, int offset, int max, std::vector<std::vector<INT_SQUARES>> & lt)
{
	std::vector<INT_SQUARES> v;
	int i=0;
	for (int square = start + offset; i != max; square += offset, i++)
	{
		const CBoard::INT_SQUARES sq = INT_SQUARES(square);
		if (off_board(sq))
			break;
		v.push_back(sq);
	}
	if (v.size())
		lt.push_back(v);
}

int CBoard::game_stage() const
{
	// The point of this is to return a number 0-100 to say how much we're in an endgame
	// A Q counts twice...
	static CHeuristicParameters chp;
	static const long max =		chp.get_value(chess::PAWN) * 16
		+	chp.get_value(chess::KNIGHT) * 4
		+	chp.get_value(chess::BISHOP) * 4
		+	chp.get_value(chess::ROOK) * 4
		+	chp.get_value(chess::QUEEN) * 4;

	// A guess at a reasonable amount of material for a typical endgame
	static const long min =		chp.get_value(chess::PAWN) * 6
		+	chp.get_value(chess::ROOK) * 1;

	long current = 0;

	for (int s = chess::WHITE; s <= chess::BLACK; s++)
	{
		for (int p = chess::PAWN; p < chess::KING; p++)
		{
			const CPiece pc(static_cast<chess::SIDE>(s), static_cast<chess::PIECE>(p));
			current += chp.get_value(pc.piece()) * piece_table()[pc].size() * (p == chess::QUEEN ? 2 : 1);
		}
	}


	return 100 - std::max(0, std::min(100, 
		static_cast<int>(static_cast<double>(current - min) / static_cast<double>(max - min) * 100.0)
		));
}

CBoard::LookupTables CBoard::createLookupTables()
{
	CBoard::LookupTables lt;
	for (int s = chess::SIDE_FIRST; s <= chess::SIDE_LAST; s += 1)
	{
		for (int p = chess::PIECE_FIRST; p <= chess::PIECE_LAST; ++p)
		{
			CPiece pc = CPiece(chess::SIDE(s),chess::PIECE(p));
			short ps = pc;
			lt[pc].resize(INT_SQUARE_LAST + 1);
			for (int sq = chess::SQUARE_FIRST; sq <= chess::SQUARE_LAST; ++sq)
			{
				const INT_SQUARES square = int_index(chess::SQUARES(sq));
				std::vector<std::vector<INT_SQUARES>> vec;
				// lt[pc][square] has the attacks;
				switch (pc.piece())
				{
				case chess::ROOK:
					AddOffsetAttacks(square, -1,  -1,vec);
					AddOffsetAttacks(square, +1,  -1,vec);
					AddOffsetAttacks(square, +16, -1,vec);
					AddOffsetAttacks(square, -16, -1,vec);
					break;							 
				case chess::BISHOP:					 
					AddOffsetAttacks(square, -17, -1,vec);
					AddOffsetAttacks(square, +17, -1,vec);
					AddOffsetAttacks(square, +15, -1,vec);
					AddOffsetAttacks(square, -15, -1,vec);
					break;							 
				case chess::QUEEN:					 
					AddOffsetAttacks(square, -15, -1,vec);
					AddOffsetAttacks(square, +15, -1,vec);
					AddOffsetAttacks(square, +17, -1,vec);
					AddOffsetAttacks(square, -17, -1,vec);
					AddOffsetAttacks(square, -1,  -1,vec);
					AddOffsetAttacks(square, +1,  -1,vec);
					AddOffsetAttacks(square, +16, -1,vec);
					AddOffsetAttacks(square, -16, -1,vec);
					break;							 
				case chess::KNIGHT:					 
					AddOffsetAttacks(square, +18, 1, vec);
					AddOffsetAttacks(square, +33, 1, vec);
					AddOffsetAttacks(square, +31, 1, vec);
					AddOffsetAttacks(square, +14, 1, vec);
					AddOffsetAttacks(square, -18, 1, vec);
					AddOffsetAttacks(square, -33, 1, vec);
					AddOffsetAttacks(square, -31, 1, vec);
					AddOffsetAttacks(square, -14, 1, vec);
					break;
				case chess::KING:
					AddOffsetAttacks(square, -15, 1, vec);
					AddOffsetAttacks(square, +15, 1, vec);
					AddOffsetAttacks(square, +17, 1, vec);
					AddOffsetAttacks(square, -17, 1, vec);
					AddOffsetAttacks(square, -1,  1, vec);
					AddOffsetAttacks(square, +1,  1, vec);
					AddOffsetAttacks(square, +16, 1, vec);
					AddOffsetAttacks(square, -16, 1, vec);
					break;
				case chess::PAWN:
					break;

				}
				lt[ps][square] = vec;
			}
		}
	}
	return lt;
}

CBoard::CBoard()
	: _side(chess::WHITE)
	, _castling(chess::CR_ALL)
	, _halfmoves(0)
	, _fullmove(1)
	, _en_passant_square(XA1)
	, _hash(0)
{
	add_piece(CPiece(chess::WHITE, chess::ROOK),		chess::A1);
	add_piece(CPiece(chess::WHITE, chess::KNIGHT),		chess::B1);
	add_piece(CPiece(chess::WHITE, chess::BISHOP),		chess::C1);
	add_piece(CPiece(chess::WHITE, chess::QUEEN),		chess::D1);
	add_piece(CPiece(chess::WHITE, chess::KING),		chess::E1);
	add_piece(CPiece(chess::WHITE, chess::BISHOP),		chess::F1);
	add_piece(CPiece(chess::WHITE, chess::KNIGHT),		chess::G1);
	add_piece(CPiece(chess::WHITE, chess::ROOK),		chess::H1);

	add_piece(CPiece(chess::WHITE, chess::PAWN),		chess::A2);
	add_piece(CPiece(chess::WHITE, chess::PAWN),		chess::B2);
	add_piece(CPiece(chess::WHITE, chess::PAWN),		chess::C2);
	add_piece(CPiece(chess::WHITE, chess::PAWN),		chess::D2);
	add_piece(CPiece(chess::WHITE, chess::PAWN),		chess::E2);
	add_piece(CPiece(chess::WHITE, chess::PAWN),		chess::F2);
	add_piece(CPiece(chess::WHITE, chess::PAWN),		chess::G2);
	add_piece(CPiece(chess::WHITE, chess::PAWN),		chess::H2);

	add_piece(CPiece(chess::BLACK, chess::ROOK),		chess::A8);
	add_piece(CPiece(chess::BLACK, chess::KNIGHT),		chess::B8);
	add_piece(CPiece(chess::BLACK, chess::BISHOP),		chess::C8);
	add_piece(CPiece(chess::BLACK, chess::QUEEN),		chess::D8);
	add_piece(CPiece(chess::BLACK, chess::KING),		chess::E8);
	add_piece(CPiece(chess::BLACK, chess::BISHOP),		chess::F8);
	add_piece(CPiece(chess::BLACK, chess::KNIGHT),		chess::G8);
	add_piece(CPiece(chess::BLACK, chess::ROOK),		chess::H8);

	add_piece(CPiece(chess::BLACK, chess::PAWN),		chess::A7);
	add_piece(CPiece(chess::BLACK, chess::PAWN),		chess::B7);
	add_piece(CPiece(chess::BLACK, chess::PAWN),		chess::C7);
	add_piece(CPiece(chess::BLACK, chess::PAWN),		chess::D7);
	add_piece(CPiece(chess::BLACK, chess::PAWN),		chess::E7);
	add_piece(CPiece(chess::BLACK, chess::PAWN),		chess::F7);
	add_piece(CPiece(chess::BLACK, chess::PAWN),		chess::G7);
	add_piece(CPiece(chess::BLACK, chess::PAWN),		chess::H7);
}

void CBoard::add_piece(CPiece p, chess::SQUARES sq) 
{
	put_piece_at(int_index(sq), p);
};

void CBoard::remove_castling_right(chess::CASTLING_RIGHTS cr)
{
	const chess::CASTLING_RIGHTS affected = chess::CASTLING_RIGHTS(_castling & cr);
	if (affected != chess::CR_NONE)
	{
		_castling = chess::CASTLING_RIGHTS(_castling & ~affected);
		_hash.ApplyCastlingRight(affected);
	}
}

CBoard::CMemento CBoard::make_move(CMove mv) 
{
	CMemento mem(mv);
	_hash.SwitchSideOnMove();

	mem.halfmove_clock = _halfmoves;

	if (_side == chess::BLACK)
		_fullmove++;


	const INT_SQUARES to = int_index(mv.to());
	const INT_SQUARES from = int_index(mv.from());

	const CPiece moved = _board[from];

	if (mv.is_capture() || moved.piece() == chess::PAWN)
	{
		_halfmoves = 0;
	}

	ASSERT(moved);

	mem.cr = _castling;
	if (_castling != chess::CR_NONE)
	{
		// Some moves may remove our castling rights
		// The capture of a rook means we can't castle there
		if (mv.is_capture() && !mv.is_en_passant_capture())
		{
			const CPiece& piece = _board[to];
			ASSERT(piece);

			if (piece.piece() == chess::ROOK) 
			{
				switch (to)
				{
				case A1: // WQ
					remove_castling_right(chess::CR_WQ);
					break;
				case A8: // BQ
					remove_castling_right(chess::CR_BQ);
					break;
				case H1: // WK
					remove_castling_right(chess::CR_WK);
					break;
				case H8: // BK
					remove_castling_right(chess::CR_BK);
					break;
				}
			}
		}
		// King moves remove all our CRs
		if (moved.piece() == chess::KING)
		{
			if (moved.side() == chess::WHITE)
			{
				remove_castling_right(chess::CR_WK | chess::CR_WQ);
			}
			else
			{
				remove_castling_right(chess::CR_BK | chess::CR_BQ);
			}
		}
		else if (moved.piece() == chess::ROOK)
		{
			switch (from)
			{
			case A1: // WQ
				remove_castling_right(chess::CR_WQ);
				break;
			case A8: // BQ
				remove_castling_right(chess::CR_BQ);
				break;
			case H1: // WK
				remove_castling_right(chess::CR_WK);
				break;
			case H8: // BK
				remove_castling_right(chess::CR_BK);
				break;
			}
		}
	}



	mem.ep = _en_passant_square;
	if (!off_board(_en_passant_square))
	{
		_hash.ApplyEnPassantFile(_en_passant_square & 7);
	}
	_en_passant_square = XA1;
	if (mv.is_easy_move())
	{
		if (mv.is_capture())
		{
			mem.captured = remove_piece_at(to);
		}

		const int dist = to - from;

		// Find out if this move allows an en passant
		// Offset will be either -32 for white or 32 for black
		if (mv.is_double_push())
		{
			const INT_SQUARES sq = INT_SQUARES(from + dist / 2);
			_en_passant_square = sq;
			_hash.ApplyEnPassantFile(sq & 7);
		}

		put_piece_at(to, remove_piece_at(from));
	}
	else if (mv.is_promotion())
	{
		if (mv.is_capture())
		{
			mem.captured = remove_piece_at(int_index(mv.to()));
		}


		const CPiece promoted(_side, mv.promotion_piece());

		const CPiece pawn = remove_piece_at(int_index(mv.from()));
		ASSERT(pawn.piece() == chess::PAWN);

		put_piece_at(int_index(mv.to()), promoted);
	}
	else if (mv.is_en_passant_capture())
	{
		// 4 . p P . . . After 1. Pc2-c4 
		// 3 . . . . . .
		// 2 . . . . . .
		// 1 . . . . . .
		//   a b c d e f

		// 4 . . . . . . black can play 1...b4xd3 
		// 3 . . p . . .
		// 2 . . . . . .
		// 1 . . . . . .
		//   a b c d e f

		// We need to remove the pawn on the square behind (-16 for white, +16 for black)
		// the 'to' square
		const INT_SQUARES en_passant_taken_square = INT_SQUARES(to - (_side == chess::WHITE ? -16 : +16));
		const CPiece taken_piece = remove_piece_at(en_passant_taken_square);
		ASSERT(taken_piece.piece() == chess::PAWN);
		//TODO ZOB FILE

		put_piece_at(to, remove_piece_at(from));
	}
	else if (mv.is_castle())
	{
		ASSERT(moved.piece() == chess::KING);
		const INT_SQUARES king_from = from; // TODO 960
		const INT_SQUARES king_to = to;

		if (_side == chess::WHITE)
		{
			remove_castling_right(chess::CR_WK | chess::CR_WQ);
		}
		else
		{
			remove_castling_right(chess::CR_BK | chess::CR_BQ);
		}

		// find the rook - 960
		INT_SQUARES rook_from, rook_to;
		if (mv.is_oo())
		{
			rook_from = oo_rook_from_square[_side];
			rook_to = oo_rook_to_square[_side];
		}
		else
		{
			rook_from = ooo_rook_from_square[_side];
			rook_to = ooo_rook_to_square[_side];
		}

		ASSERT((rook_from >> 4) == (from >> 4));
		ASSERT((rook_to >> 4) == (to >> 4));

		const CPiece rook = remove_piece_at(rook_from);
		ASSERT(rook.piece() == chess::ROOK);
		ASSERT(rook.side() == _side);
		put_piece_at(rook_to, rook);

		const CPiece king = remove_piece_at(king_from);
		ASSERT(king.piece() == chess::KING);
		ASSERT(king.side() == _side);
		put_piece_at(king_to, king);
	}
	else
	{
		ASSERT(false);
	}

	_side = (_side == chess::BLACK ? chess::WHITE : chess::BLACK);

	return mem;
};

void CBoard::put_piece_at(INT_SQUARES sq, CPiece piece)
{
	ASSERT(! _board[sq]);
	ASSERT(piece);

	_board[sq] = piece;
	_pieces[piece].insert(sq);
	_hash.ApplyPieceAtSquare(piece, index(sq));
}
CPiece CBoard::remove_piece_at(INT_SQUARES sq)
{
	ASSERT(_board[sq]);
	const CPiece piece = _board[sq];

	_board[sq] = CPiece();
	_pieces[piece].erase(sq);
	_hash.ApplyPieceAtSquare(piece, index(sq));

	return piece;
}

void CBoard::unmake_move(CMemento m) 
{
	if (_side == chess::WHITE)
		_fullmove--;
	_halfmoves = m.halfmove_clock;

	const chess::SIDE other_side = _side;

	_hash.SwitchSideOnMove();

	_side = (_side == chess::BLACK ? chess::WHITE : chess::BLACK);

	const INT_SQUARES from = int_index(m.move.from());
	const INT_SQUARES to = int_index(m.move.to());
	if (!off_board(_en_passant_square))
	{ // remove the ep square
		_hash.ApplyEnPassantFile(_en_passant_square & 7);
	}
	_en_passant_square = m.ep;
	if (!off_board(_en_passant_square))
	{ // add back the old ep square
		_hash.ApplyEnPassantFile(_en_passant_square & 7);
	}

	const chess::CASTLING_RIGHTS cr = chess::CASTLING_RIGHTS(m.cr ^ _castling);
	if (cr != chess::CR_NONE)
	{
		_hash.ApplyCastlingRight(cr);
		_castling = m.cr;
	}

	if (m.move.is_easy_move())
	{
		put_piece_at(from, remove_piece_at(to));
		if (m.captured)
			put_piece_at(to, m.captured);

	}
	else if (m.move.is_en_passant_capture())
	{
		// 4 . p P . . . After 1. Pc2-c4 
		// 3 . . . . . .
		// 2 . . . . . .
		// 1 . . . . . .
		//   a b c d e f

		// 4 . . . . . . black can play 1...b4xc3 
		// 3 . . p . . .
		// 2 . . . . . .
		// 1 . . . . . .
		//   a b c d e f

		// We need to replace the pawn on the square behind (-16 for white, +16 for black)
		// the 'to' square
		const INT_SQUARES en_passant_taken_square = INT_SQUARES(to - (_side == chess::WHITE ? -16 : +16));
		const CPiece taken_piece = CPiece(other_side, chess::PAWN);
		put_piece_at(en_passant_taken_square, taken_piece);

		put_piece_at(from, remove_piece_at(to));

	}
	else if (m.move.is_promotion())
	{
		remove_piece_at(to);
		if (m.captured)
		{
			put_piece_at(to, m.captured);
		}


		const CPiece pawn(_side, chess::PAWN);
		put_piece_at(from, pawn);
	}
	else if (m.move.is_castle())
	{
		put_piece_at(from, remove_piece_at(to));

		// find the rook - 960
		INT_SQUARES rook_from, rook_to;
		if (m.move.is_oo())
		{
			rook_from = oo_rook_from_square[_side];
			rook_to = oo_rook_to_square[_side];
		}
		else
		{
			rook_from = ooo_rook_from_square[_side];
			rook_to = ooo_rook_to_square[_side];
		}

		const CPiece rook = remove_piece_at(rook_to);
		ASSERT(rook.piece() == chess::ROOK);
		ASSERT(rook.side() == _side);
		put_piece_at(rook_from, rook);


	}
	else
	{
		ASSERT(false);
	}

};

CZobrist CBoard::hash_from_scratch() const
{
	CZobrist zob(0);
	if (_side == chess::BLACK)
		zob.SwitchSideOnMove();

	if (! off_board(_en_passant_square))
	{
		const int file07 = _en_passant_square & 7;
		zob.ApplyEnPassantFile(file07);
	}

	for (int i=0; i<8; ++i)
	{
		for (int j=0;j<8;++i)
		{
			const INT_SQUARES sq = static_cast<INT_SQUARES>(i*16 + j);
			const CPiece p = _board[sq];
			if (p)
			{
				zob.ApplyPieceAtSquare(p, index(sq));
			}
		}
	}

	return zob;
}
bool CBoard::is_square_attacked(chess::SIDE	attacker, CBoard::INT_SQUARES square) const
{
	const auto & diags = _lookups[CPiece(attacker, chess::BISHOP)][square]; // or Q or K
	const auto & horizs = _lookups[CPiece(attacker, chess::ROOK)][square];  // or Q or K
	const auto & knights = _lookups[CPiece(attacker, chess::KNIGHT)][square];
	const int white_pawns[] = { +15, +17 };
	const int black_pawns[] = { -15, -17 };

	for (const auto ray : (attacker == chess::WHITE ? white_pawns : black_pawns))
	{
		const auto piece = _board[INT_SQUARES(square + ray)];
		if (piece.side() == attacker && piece.piece() == chess::PAWN)
			return true;
	}

	for (const auto & ray : diags)
	{
		int ray_distance = 0;
		for (const auto sq : ray)
		{
			ray_distance++;
			const auto piece = _board[sq];
			if (!piece)
				continue;

			if (piece.side() == attacker && 
				((piece.piece() == chess::BISHOP || piece.piece() == chess::QUEEN)
				|| (piece.piece() == chess::KING && ray_distance == 1)))
			{
				return true;
			}

			break;
		}
	}

	for (const auto & ray : horizs)
	{
		int ray_distance = 0;
		for (const auto sq : ray)
		{
			ray_distance++;
			const auto piece = _board[sq];
			if (! piece)
				continue;
			if (piece.side() == attacker && 
				((piece.piece() == chess::ROOK || piece.piece() == chess::QUEEN)
				|| (piece.piece() == chess::KING && ray_distance == 1)))
			{
				return true;

			}
			break;
		}
	}

	for (const auto & ray : knights)
	{
		for (const auto sq : ray)
		{
			const auto piece = _board[sq];
			if (!piece)
				continue;
			if (piece.side() == attacker && 
				piece.piece() == chess::KNIGHT)
			{
				return true;
			}
		}
	}
	return false;
};

CBoard::INT_SQUARES CBoard::get_smallest_attacker(chess::SIDE attacker, CBoard::INT_SQUARES square) const
{
	const auto & diags = _lookups[CPiece(attacker, chess::BISHOP)][square]; // or Q or K
	const auto & horizs = _lookups[CPiece(attacker, chess::ROOK)][square];  // or Q or K
	const auto & knights = _lookups[CPiece(attacker, chess::KNIGHT)][square];
	const int white_pawns[] = { +15, +17 };
	const int black_pawns[] = { -15, -17 };

	// try pawns
	for (const auto ray : (attacker == chess::WHITE ? white_pawns : black_pawns))
	{
		const auto sq = INT_SQUARES(square + ray);
		const auto piece = _board[sq];
		if (piece.side() == attacker && piece.piece() == chess::PAWN)
			return sq;
	}

	// try knights
	for (const auto & ray : knights)
	{
		for (const auto sq : ray)
		{
			const auto piece = _board[sq];
			if (!piece)
				continue;
			if (piece.side() == attacker && 
				piece.piece() == chess::KNIGHT)
			{
				return sq;
			}
		}
	}

	// check diagonals. Now we might find a queen here.
	// If we do we can't be sure yet that there isn't a rook on the horizontal (or indeed a bishop on another diagonal)

	chess::PIECE smallest = chess::KING;
	INT_SQUARES smallest_sq = XA1;
	for (const auto & ray : diags)
	{
		int ray_distance = 0;
		for (const auto sq : ray)
		{
			ray_distance++;
			const auto piece = _board[sq];
			if (!piece)
				continue;

			if (piece.side() == attacker && 
				((piece.piece() == chess::BISHOP || piece.piece() == chess::QUEEN)
				|| (piece.piece() == chess::KING && ray_distance == 1)))
			{
				if (piece.piece() == chess::BISHOP)
					return sq; // a little optimisation - if we've found a bishop at this point it must be the smallest

				if (piece.piece() <= smallest)
				{
					smallest_sq = sq;
					smallest = std::min(smallest, piece.piece());
				}
			}

			break;
		}
	}

	for (const auto & ray : horizs)
	{
		int ray_distance = 0;
		for (const auto sq : ray)
		{
			ray_distance++;
			const auto piece = _board[sq];
			if (! piece)
				continue;
			if (piece.side() == attacker && 
				((piece.piece() == chess::ROOK || piece.piece() == chess::QUEEN)
				|| (piece.piece() == chess::KING && ray_distance == 1)))
			{
				if (piece.piece() == chess::ROOK)
					return sq; // a little optimisation - if we've found a rook at this point it must be the smallest

				if (piece.piece() <= smallest)
				{
					smallest_sq = sq;
					smallest = std::min(smallest, piece.piece());
				}
			}
			break;
		}
	}

	return smallest_sq;
};

int CBoard::ply() const
{
	return _halfmoves;
};
bool CBoard::is_check() const 
{
	const CPiece moving_king(side_on_move(), chess::KING);
	const chess::SIDE non_moving_side = side_on_move() == chess::WHITE ? chess::BLACK : chess::WHITE;
	const auto sq = _pieces.at(moving_king).only();
	return is_square_attacked(non_moving_side, sq);
};

chess::SIDE CBoard::side_on_move() const 
{
	return _side;
};

void CBoard::set_side_on_move(chess::SIDE s)
{
	_side = s;
};

bool CBoard::is_checkmate() 
{
	return is_check() && ! legal_moves().size(); 
};

void CBoard::try_add_move(CBoard & b, std::vector<CMove> & v, CMove mv)
{
	const auto moving_side = _side;
	const auto non_moving_side = _side == chess::WHITE ? chess::BLACK : chess::WHITE;
	{
		CBoardMutator mut(b, mv);
		const CPiece moving_king(moving_side, chess::KING);
		const auto sq = b._pieces[moving_king].only();
		if (b.is_square_attacked(non_moving_side, sq))
		{
			//nowt
		}
		else
		{
			//if (b.is_check())
			//{
			//	mv.set_flag(MOVE_CHECK); TODO
			//}
			v.push_back(mv);
		}
	}
}

void CBoard::try_add_castling_move(CBoard & b, std::vector<CMove> & v, CMove mv)
{
	// verify the intervening squares are clear
	// TODO 960
	const chess::SIDE other_side = _side == chess::WHITE ? chess::BLACK : chess::WHITE;
	ASSERT(mv.is_castle());
	if (mv.is_ooo())
	{
		// check d1/d8
		{
			const INT_SQUARES dn = int_index(chess::SQUARES(mv.to() + 1));
			if (is_occupied(dn) || is_square_attacked(other_side, dn))
				return;
		}
		// check b1/b8 does not block the rook
		{
			const INT_SQUARES dn = int_index(chess::SQUARES(mv.to() - 1));
			if (is_occupied(dn))
				return;
		}
	}
	else
	{
		// check f1/f8
		const INT_SQUARES fn = int_index(chess::SQUARES(mv.to() - 1));
		if (is_occupied(fn) || is_square_attacked(other_side, fn))
			return;
	}
	const auto to = int_index(mv.to());
	if (is_occupied(to) || is_square_attacked(other_side, to))
		return;

	try_add_move(b, v, mv);
}

std::vector<CMove> CBoard::legal_moves_q() 
{
	std::vector<CMove> v;
	v.reserve(20);
	const chess::SIDE other_side = (_side == chess::WHITE ? chess::BLACK : chess::WHITE);
	const chess::CASTLING_RIGHTS my_castling = 
		chess::CASTLING_RIGHTS(_castling &
		(_side == chess::WHITE ? (chess::CR_WK | chess::CR_WQ) : (chess::CR_BK | chess::CR_BQ)));

	if (!_pieces[CPiece(_side, chess::KING)].size())
		return v;

	CBoard b2(*this);

	for (int px = chess::PIECE_FIRST; px <= chess::PIECE_LAST; ++ px)
	{

		const CPiece thePiece(_side, static_cast<chess::PIECE>(px));
		const CPieceList& theList = _pieces[thePiece];

		if (theList.empty()) continue;

		//if (thePiece.piece() == chess::KING 
		//	&& my_castling != chess::CR_NONE
		//	&& !b2.is_check())
		//{

		//	// castling
		//	const INT_SQUARES k = theList.only();
		//	const INT_SQUARES home_square = (_side == chess::WHITE ? E1 : E8);
		//	if (k == home_square)
		//	{
		//		// TODO 960

		//		if (my_castling)
		//		{
		//			if (my_castling & chess::CR_WQ)
		//				try_add_castling_move(b2, v, CMove(chess::E1, chess::C1, MOVE_OOO));
		//			if (my_castling & chess::CR_BQ)
		//				try_add_castling_move(b2, v, CMove(chess::E8, chess::C8, MOVE_OOO));
		//			if (my_castling & chess::CR_WK)
		//				try_add_castling_move(b2, v, CMove(chess::E1, chess::G1, MOVE_OO));
		//			if (my_castling & chess::CR_BK)
		//				try_add_castling_move(b2, v, CMove(chess::E8, chess::G8, MOVE_OO));
		//		}
		//	}
		//}


		switch (thePiece.piece())
		{
		case chess::ROOK:
		case chess::KNIGHT:
		case chess::BISHOP:
		case chess::QUEEN:
		case chess::KING:
			{
				for (const auto & from : theList)
				{
					for (const auto & ray : _lookups[thePiece][from])
					{
						for (auto to : ray)
						{
							const CPiece atSquare = _board[to];

							if (atSquare > 0)
							{
								// poss capture
								if (atSquare.side() == other_side)
								{
									try_add_move(b2, v, CMove(index(from), index(to), MOVE_CAPTURE));
								}
								break;
							}
						}
					}
				}
			}
			break;
		case chess::PAWN:
			{
				// Direction of pawn travel depends on side
				const int direction = (_side == chess::WHITE ? -16 : 16);
				// Home rank depends on side
				const int homeRank = (_side == chess::WHITE ? 6 : 1);
				// Destination rank depends on side
				const int destRank = (_side == chess::WHITE ? 0 : 7);

				for_each(theList.begin(), theList.end(), [&](const INT_SQUARES from)
				{

					{
						const INT_SQUARES new_to = INT_SQUARES(from + direction);
						if (! off_board(new_to) && !is_occupied(new_to))
						{
							// Test promotion
							if ((new_to >> 4) == destRank)
							{
								const chess::PIECE promotablePieces[] = {chess::QUEEN, chess::KNIGHT, chess::ROOK, chess::BISHOP};
								for (chess::PIECE p : promotablePieces)
									try_add_move(b2, v, CMove(index(from), index(new_to), MOVE_PROMOTION, p));
							}
						}
					}

					{
						// 2x captures, direction +/- 1
						for (int to = from + direction - 1; to <= from + direction + 1; to += 2) {
							const INT_SQUARES new_to = INT_SQUARES(to);
							const CPiece atSquare = _board[new_to];
							if (! off_board(new_to))
							{
								if (atSquare && atSquare.side() != _side)
								{
									if ((new_to >> 4) == destRank)
									{
										const chess::PIECE promotablePieces[] = {chess::QUEEN, chess::KNIGHT, chess::ROOK, chess::BISHOP};
										for (chess::PIECE p : promotablePieces)
											try_add_move(b2, v, CMove(index(from), index(new_to), MOVE_CAPTURE | MOVE_PROMOTION, p));
									}
									else
									{
										try_add_move(b2, v, CMove(index(from), index(new_to), MOVE_CAPTURE));
									}
								}

								if (_en_passant_square != XA1)
								{
									if (new_to == _en_passant_square)
									{
										try_add_move(b2, v, CMove(index(from), index(new_to), MOVE_FLAGS(MOVE_EN_PASSANT | MOVE_CAPTURE)));
									}
								}
							}
						}
					}

				}); // end piece loop

			} // end pawn case
			break;
		}
	}

	return v;
}

std::vector<CMove> CBoard::legal_moves() 
{
	std::vector<CMove> v;
	v.reserve(100);
	const chess::SIDE the_other_side = other_side(_side);
	const chess::CASTLING_RIGHTS my_castling = 
		chess::CASTLING_RIGHTS(_castling &
		(_side == chess::WHITE ? (chess::CR_WK | chess::CR_WQ) : (chess::CR_BK | chess::CR_BQ)));

	CBoard b2(*this);

	for (int px = chess::PIECE_FIRST; px <= chess::PIECE_LAST; ++ px)
	{
		const CPiece thePiece(_side, static_cast<chess::PIECE>(px));
		const CPieceList& theList = _pieces[thePiece];

		if (theList.empty()) continue;
		if (thePiece.piece() == chess::KING 
			&& my_castling != chess::CR_NONE
			&& !b2.is_check())
		{

			// castling
			const INT_SQUARES k = theList.only();
			const INT_SQUARES home_square = (_side == chess::WHITE ? E1 : E8);
			if (k == home_square)
			{
				// TODO 960

				if (my_castling)
				{
					if (my_castling & chess::CR_WQ)
						try_add_castling_move(b2, v, CMove(chess::E1, chess::C1, MOVE_OOO));
					if (my_castling & chess::CR_BQ)
						try_add_castling_move(b2, v, CMove(chess::E8, chess::C8, MOVE_OOO));
					if (my_castling & chess::CR_WK)
						try_add_castling_move(b2, v, CMove(chess::E1, chess::G1, MOVE_OO));
					if (my_castling & chess::CR_BK)
						try_add_castling_move(b2, v, CMove(chess::E8, chess::G8, MOVE_OO));
				}
			}
		}

		switch (thePiece.piece())
		{
		case chess::ROOK:
		case chess::KNIGHT:
		case chess::BISHOP:
		case chess::QUEEN:
		case chess::KING:
			{
				for (const auto & from : theList)
				{
					for (const auto & ray : _lookups[thePiece][from])
					{
						for (auto to : ray)
						{
							const CPiece atSquare = _board[to];

							if (atSquare > 0)
							{
								// poss capture
								if (atSquare.side() == the_other_side)
								{
									try_add_move(b2, v, CMove(index(from), index(to), MOVE_CAPTURE));
								}
								break;
							}
							try_add_move(b2, v, CMove(index(from), index(to), MOVE_NONE));
						}
					}
				}
			}
			break;
		case chess::PAWN:
			{
				// Direction of pawn travel depends on side
				const int direction = (_side == chess::WHITE ? -16 : 16);
				// Home rank depends on side
				const int homeRank = (_side == chess::WHITE ? 6 : 1);
				// Destination rank depends on side
				const int destRank = (_side == chess::WHITE ? 0 : 7);

				for_each(theList.begin(), theList.end(), [&](const INT_SQUARES from)
				{

					{
						const INT_SQUARES new_to = INT_SQUARES(from + direction);
						if (! off_board(new_to) && !is_occupied(new_to))
						{
							// Test promotion
							if ((new_to >> 4) == destRank)
							{
								const chess::PIECE promotablePieces[] = {chess::QUEEN, chess::KNIGHT, chess::ROOK, chess::BISHOP};
								for (chess::PIECE p : promotablePieces)
									try_add_move(b2, v, CMove(index(from), index(new_to), MOVE_PROMOTION, p));
							}
							else
							{
								try_add_move(b2, v, CMove(index(from), index(new_to), MOVE_NONE));

								// If on home rank we can move 2 squares as well, provided not blocked
								if ((from >> 4) == homeRank)
								{
									const INT_SQUARES new_to = INT_SQUARES(from + 2*direction);
									if (!is_occupied(new_to))
									{
										try_add_move(b2, v, CMove(index(from), index(new_to), MOVE_DBL_PUSH));
									}
								}
							}
						}
					}

					{
						// 2x captures, direction +/- 1
						for (int to = from + direction - 1; to <= from + direction + 1; to += 2) {
							const INT_SQUARES new_to = INT_SQUARES(to);
							const CPiece atSquare = _board[new_to];
							if (! off_board(new_to))
							{
								if (atSquare && atSquare.side() != _side)
								{
									if ((new_to >> 4) == destRank)
									{
										const chess::PIECE promotablePieces[] = {chess::QUEEN, chess::KNIGHT, chess::ROOK, chess::BISHOP};
										for (chess::PIECE p : promotablePieces)
											try_add_move(b2, v, CMove(index(from), index(new_to), MOVE_CAPTURE | MOVE_PROMOTION, p));
									}
									else
									{
										try_add_move(b2, v, CMove(index(from), index(new_to), MOVE_CAPTURE));
									}
								}

								if (_en_passant_square != XA1)
								{
									if (new_to == _en_passant_square)
									{
										try_add_move(b2, v, CMove(index(from), index(new_to), MOVE_FLAGS(MOVE_EN_PASSANT | MOVE_CAPTURE)));
									}
								}
							}
						}
					}

				}); // end piece loop

			} // end pawn case
			break;
		}
	}

	return v;
};

CPiece CBoard::piece_at_square(chess::SQUARES sq) const 
{
	return _board[int_index(sq)];
};

CPiece CBoard::piece_at_square(INT_SQUARES sq) const 
{
	return _board[sq];
};

std::string CBoard::san_name(CMove m) const 
{

	std::stringstream ss;

	if (m.is_oo())
	{
		ss << "O-O";
	}
	else if (m.is_ooo())
	{
		ss << "O-O-O";
	}
	else
	{

		// what kind of piece is moving?
		const CPiece moving = piece_at_square(m.from());
		switch (moving.piece())
		{
		case chess::PAWN:
			// if a capture, only specify file
			if (m.is_capture())
			{
				ss << chess::square_file_char(m.from());
			}
			break;
		default:
			ss << moving.as_string();
			// How many such pieces can move to this square
			std::vector<INT_SQUARES> possibles;
			for (const auto sq : _pieces[moving])
			{
				if (piece_attacks_square(moving, sq, int_index(m.to())))
					possibles.push_back(sq);
			}
			if (possibles.size() == 0)
			{
				ASSERT(false);
			}
			else if (possibles.size() > 1)
			{
				// Disambiguate the square somehow
				std::set<char> files, ranks;
				transform(possibles.begin(), possibles.end(), inserter(files, files.end()), [](INT_SQUARES sq)
				{
					return chess::square_file_char(index(sq));
				});
				transform(possibles.begin(), possibles.end(), inserter(ranks, ranks.end()), [](INT_SQUARES sq)
				{
					return chess::square_rank_char(index(sq));
				});

				if (files.size() == possibles.size())
				{ // File is sufficient to distinguish
					ss << chess::square_file_char(m.from());
				}
				else
				{
					if (ranks.size() == possibles.size())
					{ // Rank suffices
						ss << chess::square_rank_char(m.from());
					}
					else
					{ // Full square name
						ss << chess::name_of_square(m.from());
					}
				}
			}
			break;
		}

		// capture?
		if (m.is_capture())
		{
			ss << "x";
		}

		// to square
		ss << chess::name_of_square(m.to());

		// promotion?
		if (m.is_promotion())
		{
			ss << "=" << chess::PIECE_STRINGS[m.promotion_piece()];
		}
	}
	// check?
	if (m.is_check())
	{
		ss << "+";
	}
	return ss.str();
};

bool CBoard::is_occupied(const CBoard::INT_SQUARES sq) const
{
	return _board[sq].piece() != chess::NOTHING;
}

void CBoard::clear_board()
{
	for (int i = 0; i<128; i++)
	{
		_board[i] = CPiece();
	}
	_pieces.clear();
}

bool CBoard::piece_attacks_square(const CPiece piece, const INT_SQUARES on, const INT_SQUARES target)const
{
	ASSERT(piece.piece() != chess::PAWN);
	for (const auto & ray : _lookups[piece][on])
	{
		for (const auto & sq : ray)
		{
			CPiece here = piece_at_square(sq);
			if (here)
			{
				if (here.side() != piece.side())
				{
					if (sq == target)
					{
						return true;
					}
				}
				break;
			}
			if (sq == target)
				return true;
		}
	}
	return false;
}


void CBoard::set_fen_position(std::string fen)
{
	boost::algorithm::trim(fen);
	if (fen == "1")
	{
		set_fen_position("2bqkbn1/2pppp2/np2N3/r3P1p1/p2N2B1/5Q2/PPPPKPP1/RN_br2 w KQkq - 0 1");
		return;
	}
	if (fen == "P2")
	{
		set_fen_position("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
		return;
	}
	if (fen == "P3")
	{
		set_fen_position("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
		return;
	}
	if (fen == "P4")
	{
		set_fen_position("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
		return;
	}
	if (fen == "P5")
	{
		set_fen_position("rnbqkb1r/pp1p1ppp/2p5/4P3/2B5/8/PPP1NnPP/RNBQK2R w KQkq - 0 6");
		return;
	}


	clear_board();

	boost::char_separator<char> sep(" /");
	boost::tokenizer<boost::char_separator<char>> toks(fen, sep);

	auto tok = toks.begin();
	for (int row = 0; row < 8; row++)
	{
		int col = 0;
		for (auto c : *tok)
		{
			if (c >= '1' && c <= '8')
			{
				col += c - '0';
			}
			else
			{
				const auto piece = CPiece::from_char(c);
				add_piece(piece, chess::SQUARES(row * 8 + col));
				col++;
			}
		}
		tok++;
	}

	if (tok.at_end())
		return;

	// side on move

	if (*tok == "b")
	{
		set_side_on_move(chess::BLACK);
		_hash.SwitchSideOnMove();
	}
	else
	{
		set_side_on_move(chess::WHITE);
	}
	tok++;
	if (tok.at_end())
		return;

	// castling rights


	_castling = chess::CR_NONE;
	for (char c : *tok)
	{
		switch (c)
		{
		case 'K': _castling = _castling | chess::CR_WK; break;
		case 'Q': _castling = _castling | chess::CR_WQ; break;
		case 'k': _castling = _castling | chess::CR_BK; break;
		case 'q': _castling = _castling | chess::CR_BQ; break;
		}
	}

	tok++;
	if (tok.at_end())
		return;

	// ep
	if (*tok != "-")
	{
		_en_passant_square = int_index(chess::square_from_string(*tok));
	}
	else
	{
		_en_passant_square = XA1;
	}
	tok++;
	if (tok.at_end())
		return;

	// halfmove clock
	_halfmoves = atoi(tok->c_str());
	tok++;
	if (tok.at_end())
		return;

	// fullmove number
	_fullmove = atoi(tok->c_str());
	tok++;
	ASSERT (tok.at_end());

	ASSERT(fen == this->fen());
}
bool CBoard::assert_piecelist_consistent() const
{
	CBoard _b;
	_b.set_fen_position(fen());
	return _b._pieces.equals(_pieces);
};

std::string CBoard::fen() const 
{
	std::stringstream ss;
	for (int row = 0; row < 8; row++)
	{
		int empty = 0;
		for (int col = 0; col < 8; col++)
		{
			const INT_SQUARES sq = INT_SQUARES(row * 16 + col);
			if (_board[sq].piece() != chess::NOTHING)
			{
				if (empty > 0)
				{
					ss << empty;
					empty = 0;
				}
				ss << _board[sq].as_side_string();
			}
			else
			{
				empty++;
			}
		}

		if (empty > 0)
		{ 
			ss << empty;
			empty = 0;
		}
		if (row < 7)
			ss << "/";
	}

	ss << " ";
	// side on move

	ss << (_side == chess::WHITE ? "w" : "b");
	ss << " ";

	// castling rights

	if (_castling != chess::CR_NONE)
	{
		if (_castling & chess::CR_WK) ss << "K";
		if (_castling & chess::CR_WQ) ss << "Q";
		if (_castling & chess::CR_BK) ss << "k";
		if (_castling & chess::CR_BQ) ss << "q";
	}
	else
	{
		ss << "-";
	}

	// ep square
	ss << " ";
	if (_en_passant_square != XA1)
	{
		ss << name_of_square(index(_en_passant_square));
	}
	else
	{
		ss << "-";
	}

	ss << " ";
	ss << _halfmoves;
	ss << " ";
	ss << _fullmove;


	return ss.str();
};

std::string CBoard::board() const 
{
	std::stringstream ss;
	for (int row = 0; row < 8; row++)
	{
		for (int col = 0; col < 8; col++)
		{
			const INT_SQUARES sq = INT_SQUARES(row * 16 + col);
			if (_board[sq].piece() != chess::NOTHING)
			{
				ss << " " << _board[sq].as_side_string();
			}
			else
			{
				ss << " .";
			}
		}
		if ((row == 0 && _side == chess::BLACK) || (row == 7 && _side == chess::WHITE))
			ss << " *";
		ss << std::endl;
	}
	return ss.str();
};

CZobrist CBoard::hash() const 
{
	return _hash;
};
