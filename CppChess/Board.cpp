#include "stdafx.h"
#include "Board.h"
#include "BoardMutator.h"

namespace
{
	CBoard::INT_SQUARES int_index(chess::SQUARES sq)
	{
		return CBoard::INT_SQUARES(sq + (sq & ~7));
	}

	chess::SQUARES index(CBoard::INT_SQUARES sq)
	{
		const int row = (sq / 16);
		const int col = (sq % 16);
		ASSERT(col < 8);
		return chess::SQUARES((row * 8) + col);
	}

	bool off_board(CBoard::INT_SQUARES sq)
	{
		return (sq & 0x88) != 0;
	}

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

CBoard::LookupTables CBoard::createLookupTables()
{
	CBoard::LookupTables lt;
	for (int s = chess::SIDE_FIRST; s <= chess::SIDE_LAST; s += 1)
	{
		for (int p = chess::PIECE_FIRST; p <= chess::PIECE_LAST; ++p)
		{
			CPiece pc = CPiece(chess::SIDE(s),chess::PIECE(p));

			for (int sq = chess::SQUARE_FIRST; sq <= chess::SQUARE_LAST; ++sq)
			{
				const INT_SQUARES square = int_index(chess::SQUARES(sq));
				// lt[pc][square] has the attacks;
				switch (pc.piece())
				{
				case chess::ROOK:
					AddOffsetAttacks(square, -1,  -1, lt[pc][square]);
					AddOffsetAttacks(square, +1,  -1, lt[pc][square]);
					AddOffsetAttacks(square, +16, -1, lt[pc][square]);
					AddOffsetAttacks(square, -16, -1, lt[pc][square]);
					break;
				case chess::BISHOP:
					AddOffsetAttacks(square, -17, -1, lt[pc][square]);
					AddOffsetAttacks(square, +17, -1, lt[pc][square]);
					AddOffsetAttacks(square, +15, -1, lt[pc][square]);
					AddOffsetAttacks(square, -15, -1, lt[pc][square]);
					break;
				case chess::QUEEN:
					AddOffsetAttacks(square, -15, -1, lt[pc][square]);
					AddOffsetAttacks(square, +15, -1, lt[pc][square]);
					AddOffsetAttacks(square, +17, -1, lt[pc][square]);
					AddOffsetAttacks(square, -17, -1, lt[pc][square]);
					AddOffsetAttacks(square, -1,  -1, lt[pc][square]);
					AddOffsetAttacks(square, +1,  -1, lt[pc][square]);
					AddOffsetAttacks(square, +16, -1, lt[pc][square]);
					AddOffsetAttacks(square, -16, -1, lt[pc][square]);
					break;
				case chess::KNIGHT:
					AddOffsetAttacks(square, +18, 1, lt[pc][square]);
					AddOffsetAttacks(square, +33, 1, lt[pc][square]);
					AddOffsetAttacks(square, +31, 1, lt[pc][square]);
					AddOffsetAttacks(square, +14, 1, lt[pc][square]);
					AddOffsetAttacks(square, -18, 1, lt[pc][square]);
					AddOffsetAttacks(square, -33, 1, lt[pc][square]);
					AddOffsetAttacks(square, -31, 1, lt[pc][square]);
					AddOffsetAttacks(square, -14, 1, lt[pc][square]);

					//  0  1  2  3
					// 16 17 18 19
					// 32 33 34 35
					// 48 49 50 51
					break;
				case chess::KING:
					AddOffsetAttacks(square, -15, 1, lt[pc][square]);
					AddOffsetAttacks(square, +15, 1, lt[pc][square]);
					AddOffsetAttacks(square, +17, 1, lt[pc][square]);
					AddOffsetAttacks(square, -17, 1, lt[pc][square]);
					AddOffsetAttacks(square, -1,  1, lt[pc][square]);
					AddOffsetAttacks(square, +1,  1, lt[pc][square]);
					AddOffsetAttacks(square, +16, 1, lt[pc][square]);
					AddOffsetAttacks(square, -16, 1, lt[pc][square]);
					break;
				case chess::PAWN:
					break;

				}
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
	const INT_SQUARES i = int_index(sq);
	_board[i] = p;
	_pieces[p].insert(i);
	_hash.ApplyPieceAtSquare(p, sq);
};

CBoard::CMemento CBoard::make_move(CMove mv) 
{
	CMemento mem(mv);
	mem.hash.SwitchSideOnMove();

	if (_side == chess::BLACK)
		_fullmove++;
	_halfmoves++;

	const INT_SQUARES to = int_index(mv.to());
	const INT_SQUARES from = int_index(mv.from());

	const CPiece moved = _board[from];

	mem.cr = _castling;
	mem.ep = _en_passant_square;
	_en_passant_square = XA1;
	if (mv.is_normal_move())
	{
		if (mv.is_capture())
		{
			mem.captured = piece_at_square(mv.to());
			mem.hash.ApplyPieceAtSquare(mem.captured, mv.to());
			_pieces[mem.captured].erase(int_index(mv.to()));
		}

		const int dist = to - from;

		// Find out if this move allows an en passant
		// i.e. is this a double pawn push
		// Offset will be either -32 for white or 32 for black
		if (mv.is_double_push())
		{
			const INT_SQUARES sq = INT_SQUARES(from + dist / 2);
			_en_passant_square = sq;
			// TODO mem.hash.ApplyEnPassantFile(sq & 7);
		}

		{
			// Move the bits
			_pieces[moved].erase(from);
			mem.hash.ApplyPieceAtSquare(moved, index(from));
			_pieces[moved].insert(to);
			mem.hash.ApplyPieceAtSquare(moved, index(to));

			_board[to] = _board[from];
			_board[from] = CPiece();
		}
	}
	else if (mv.is_promotion())
	{
		if (mv.is_capture())
		{
			mem.captured = piece_at_square(mv.to());
			mem.hash.ApplyPieceAtSquare(mem.captured, mv.to());
			_pieces[mem.captured].erase(int_index(mv.to()));
		}

		const CPiece promoted(_side, mv.promotion_piece());
		const CPiece pawn(_side, chess::PAWN);
		_pieces[promoted].insert(to);
		mem.hash.ApplyPieceAtSquare(promoted, index(to));
		_pieces[pawn].erase(from);
		mem.hash.ApplyPieceAtSquare(pawn, index(from));
		
		_board[to] = promoted;
		_board[from] = CPiece();
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
		const INT_SQUARES en_passant_taken_square = INT_SQUARES(int_index(mv.to()) - (_side == chess::WHITE ? -16 : +16));
		const CPiece taken_piece = _board[en_passant_taken_square];
		ASSERT(taken_piece.piece() == chess::PAWN);
		_pieces[taken_piece].erase(en_passant_taken_square);
		mem.hash.ApplyPieceAtSquare(taken_piece, index(en_passant_taken_square));

		//TODO ZOB FILE

		_board[en_passant_taken_square] = CPiece();

		_pieces[moved].erase(from);
		mem.hash.ApplyPieceAtSquare(moved, index(from));

		_pieces[moved].insert(to);
		mem.hash.ApplyPieceAtSquare(moved, index(to));

		_board[to] = _board[from];
		_board[from] = CPiece();
	}
	else if (mv.is_castle())
	{
		ASSERT(moved.piece() == chess::KING);
		const INT_SQUARES king_from = from; // TODO 960
		const INT_SQUARES king_to = to;

		// find the rook - 960
		INT_SQUARES rook_from, rook_to;
		if (mv.is_oo())
		{
			if (_side == chess::WHITE)
			{
				rook_from = H1;
				rook_to = F1;
			}
			else
			{
				rook_from = H8;
				rook_to = F8;
			}
		}
		else
		{
			if (_side == chess::WHITE)
			{
				rook_from = A1;
				rook_to = D1;
			}
			else
			{
				rook_from = A8;
				rook_to = D8;
			}
		}

		const CPiece rook = _board[rook_from];
		ASSERT(rook.piece() == chess::ROOK);
		ASSERT(rook.side() == _side);

		_pieces[rook].erase(rook_from);
		mem.hash.ApplyPieceAtSquare(rook, index(rook_from));
		_pieces[rook].insert(rook_to);
		mem.hash.ApplyPieceAtSquare(rook, index(rook_to));

		_board[rook_to] = _board[rook_from];
		_board[rook_from] = CPiece();

		_pieces[moved].erase(king_from);
		mem.hash.ApplyPieceAtSquare(moved, index(king_from));
		_pieces[moved].insert(king_to);
		mem.hash.ApplyPieceAtSquare(moved, index(king_to));

		_board[king_to] = _board[king_from];
		_board[king_from] = CPiece();
	}

	_side = (_side == chess::BLACK ? chess::WHITE : chess::BLACK);

	_hash.Apply(mem.hash);
	return mem;
};

void CBoard::unmake_move(CMemento m) 
{
	_hash.Apply(m.hash);

	if (_side == chess::WHITE)
		_fullmove--;
	_halfmoves--;

	const chess::SIDE other_side = _side;

	_side = (_side == chess::BLACK ? chess::WHITE : chess::BLACK);

	const INT_SQUARES from = int_index(m.move.from());
	const INT_SQUARES to = int_index(m.move.to());
	const CPiece moved = _board[to];
	_en_passant_square = m.ep;
	_castling = m.cr;

	if (m.move.is_normal_move())
	{
		_board[from] = moved;
		_board[to] = m.captured;

		if (m.captured.piece() != chess::NOTHING)
		{
			_pieces[m.captured].insert(to);
		}

		_pieces[moved].erase(to);
		_pieces[moved].insert(from);
	}
	else if (m.move.is_promotion())
	{
		_board[to] = m.captured;
		if (m.captured.piece() != chess::NOTHING)
		{
			_pieces[m.captured].insert(to);
		}

		const CPiece promoted(_side, m.move.promotion_piece());
		const CPiece pawn(_side, chess::PAWN);
		_pieces[promoted].erase(to);
		_pieces[pawn].insert(from);
		_board[from] = pawn;
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

		_pieces[taken_piece].insert(en_passant_taken_square);
		_board[en_passant_taken_square] = taken_piece;
	}
	else if (m.move.is_castle())
	{
		ASSERT(moved.piece() == chess::KING);
		// find the rook - 960
		INT_SQUARES rook_from, rook_to;
		if (m.move.is_oo())
		{
			if (_side == chess::WHITE)
			{
				rook_from = H1;
				rook_to = F1;
			}
			else
			{
				rook_from = H8;
				rook_to = F8;
			}
		}
		else
		{
			if (_side == chess::WHITE)
			{
				rook_from = A1;
				rook_to = D1;
			}
			else
			{
				rook_from = A8;
				rook_to = D8;
			}
		}

		const CPiece rook = _board[rook_to];
		ASSERT(rook.piece() == chess::ROOK);
		ASSERT(rook.side() == _side);

		_pieces[rook].insert(rook_from);
		_pieces[rook].erase(rook_to);


		_board[rook_from] = _board[rook_to];
		_board[rook_to] = CPiece();
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
	CBoardMutator mut(b, mv);
	const CPiece moving_king(moving_side, chess::KING);
	const auto sq = b._pieces[moving_king].only();
	if (b.is_square_attacked(non_moving_side, sq))
	{
		//nowt
	}
	else
	{
		if (b.is_check())
		{
			mv.set_flag(MOVE_CHECK);
		}
		v.push_back(mv);
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
		const INT_SQUARES dn = int_index(chess::SQUARES(mv.to() + 1));
		if (is_occupied(dn) || is_square_attacked(other_side, dn))
			return;
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

std::vector<CMove> CBoard::legal_moves() 
{
	std::vector<CMove> v;
	v.reserve(40);
	const chess::SIDE other_side = (_side == chess::WHITE ? chess::BLACK : chess::WHITE);

	CBoard b2(*this);
	for (int px = chess::PIECE_FIRST; px <= chess::PIECE_LAST; ++ px)
	{
		const CPiece thePiece(_side, static_cast<chess::PIECE>(px));
		const CPieceList& theList = _pieces[thePiece];

		if (theList.empty()) continue;
		if (thePiece.piece() == chess::KING)
		{
			// castling
			const INT_SQUARES k = *theList.begin();
			const INT_SQUARES home_square = (_side == chess::WHITE ? E1 : E8);
			if (k == home_square)
			{
				// TODO 960
				const chess::CASTLING_RIGHTS my_castling = 
					chess::CASTLING_RIGHTS(_castling &
					(_side == chess::WHITE ? (chess::CR_WK | chess::CR_WQ) : (chess::CR_BK | chess::CR_BQ)));

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
								if (atSquare.side() == other_side)
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

std::string CBoard::san_name(CMove m) const 
{
	return std::string();
};

bool CBoard::is_occupied(CBoard::INT_SQUARES sq) const
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

void CBoard::set_fen_position(std::string fen)
{

	if (fen == "1")
	{
		set_fen_position("2bqkbn1/2pppp2/np2N3/r3P1p1/p2N2B1/5Q2/PPPPKPP1/RNB2r2 w KQkq - 0 1");
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
	CBoard b2;
	b2.set_fen_position(fen());
	return b2._pieces.equals(_pieces);
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

uint64_t CBoard::hash() const 
{
	return _hash.Hash();
};
