#include "stdafx.h"
#include "Board.h"

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
	for (int s = chess::SIDE_FIRST; s <= chess::SIDE_LAST; s += 16)
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
};

CMemento CBoard::make_move(CMove mv) 
{
	CMemento mem(mv);
	if (mv.is_capture())
	{
		mem.captured = piece_at_square(mv.to());
		_pieces[mem.captured].erase(int_index(mv.to()));
	}

	if (_side == chess::BLACK)
		_fullmove++;
	_halfmoves++;

	_side = (_side == chess::BLACK ? chess::WHITE : chess::BLACK);

	//
	const INT_SQUARES to = int_index(mv.to());
	const INT_SQUARES from = int_index(mv.from());

	const CPiece moved = _board[from];

	_pieces[moved].erase(from);
	_pieces[moved].insert(to);

	_board[to] = _board[from];
	_board[from] = CPiece();

	return mem;
};

void CBoard::unmake_move(CMemento m) 
{
	if (_side == chess::WHITE)
		_fullmove--;
	_halfmoves--;

	_side = (_side == chess::BLACK ? chess::WHITE : chess::BLACK);

	const INT_SQUARES from = int_index(m.move.from());
	const INT_SQUARES to = int_index(m.move.to());
	const CPiece moved = _board[to];

	_board[from] = moved;
	_board[to] = m.captured;

	if (m.captured.piece() != chess::NOTHING)
	{
		_pieces[m.captured].insert(to);
	}

	_pieces[moved].erase(to);
	_pieces[moved].insert(from);
};

bool CBoard::is_check() const 
{
	return false; //TODO
};

bool CBoard::is_checkmate() const 
{
	return false; 
};

std::vector<CMove> CBoard::legal_moves() const 
{
	std::vector<CMove> m;
	m.reserve(20);
	for (const auto & piece : _pieces)
	{
		if (piece.first.side() != _side)
			continue;

		switch (piece.first.piece())
		{
		case chess::ROOK:
		case chess::KNIGHT:
		case chess::BISHOP:
		case chess::QUEEN:
		case chess::KING:
			{
				for (const auto & from : piece.second)
				{
					for (const auto & ray : _lookups[piece.first][from])
					{
						for (auto to : ray)
						{
							if (is_occupied(to))
							{
								// poss capture
								if (_board[to].side() != _side)
								{
									m.push_back(CMove(index(from), index(to), CMove::MOVE_CAPTURE));
								}
								break;
							}
							m.push_back(CMove(index(from), index(to), CMove::MOVE_NONE));
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

				for (const auto & from : piece.second)
				{
					// If on home rank we can move 2 squares as well, provided not blocked
					if ((from >> 4) == homeRank)
					{
						const INT_SQUARES new_to = INT_SQUARES(from + 2*direction);
						if (!is_occupied(new_to))
						{
							m.push_back(CMove(index(from), index(new_to), CMove::MOVE_NONE));
							// TODO set EP square.
						}
					}

					{
						const INT_SQUARES new_to = INT_SQUARES(from + direction);
						if (!is_occupied(new_to))
						{
							m.push_back(CMove(index(from), index(new_to), CMove::MOVE_NONE));
						}
					}

					// TODO Captures
					// TODO promotions
					// TODO en passant
			
				}

			}
			break;
		}
	}
	return m;
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
	return ss.str();
};

long CBoard::hash() const 
{
	return 0;
};
