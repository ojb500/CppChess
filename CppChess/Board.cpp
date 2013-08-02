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

CBoard::CMemento CBoard::make_move(CMove mv) 
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


	const INT_SQUARES to = int_index(mv.to());
	const INT_SQUARES from = int_index(mv.from());

	const CPiece moved = _board[from];



	const int dist = from - to;

	mem.ep = _en_passant_square;

	// Find out if this move allows an en passant
	// i.e. is this a double pawn push
	// Offset will be either -32 for white or 32 for black
	if (mv.is_double_push() && (dist == -32 || dist == 32))
	{
		_en_passant_square = INT_SQUARES(from + dist / 2);
	}
	else
	{
		_en_passant_square.reset();
	}

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

	_en_passant_square = m.ep;
	_castling = m.cr;

	_pieces[moved].erase(to);
	_pieces[moved].insert(from);
};

bool CBoard::is_square_attacked(chess::SIDE	attacker, CBoard::INT_SQUARES square) const
{
	// TODO pawn attacks
	const auto & diags = _lookups[CPiece(attacker, chess::BISHOP)][square]; // or Q or K
	const auto & horizs = _lookups[CPiece(attacker, chess::ROOK)][square];  // or Q or K
	const auto & knights = _lookups[CPiece(attacker, chess::KNIGHT)][square];

	for (const auto & ray : diags)
	{
		int ray_distance = 0;
		for (const auto sq : ray)
		{
			ray_distance++;
			const auto piece = _board[sq];
			switch(piece.side())
			{
			case chess::NONE:
				continue;
			default:
				{
					if (piece.side() == attacker && 
						((piece.piece() == chess::BISHOP || piece.piece() == chess::QUEEN)
						|| (piece.piece() == chess::KING && ray_distance == 1)))
					{
						return true;
					}
					else
					{
						break;
					}
				}
			}
		}
	}

	for (const auto & ray : horizs)
	{
		int ray_distance = 0;
		for (const auto sq : ray)
		{
			ray_distance++;
			const auto piece = _board[sq];
			switch(piece.side())
			{
			case chess::NONE:
				continue;
			default:
				{
					if (piece.side() == attacker && 
						((piece.piece() == chess::ROOK || piece.piece() == chess::QUEEN)
						|| (piece.piece() == chess::KING && ray_distance == 1)))
					{
						return true;
					}
					else
					{
						break;
					}
				}
			}
		}
	}

	for (const auto & ray : knights)
	{
		for (const auto sq : ray)
		{
			const auto piece = _board[sq];
			switch(piece.side())
			{
			case chess::NONE:
				continue;
			default:
				{
					if (piece.side() == attacker && 
						piece.piece() == chess::KNIGHT)
					{
						return true;
					}
					else
					{
						break;
					}
				}
			}
		}
	}
	// TODO pawn attacks
	return false;

};

bool CBoard::is_check() const 
{
	const CPiece moving_king(side_on_move(), chess::KING);
	const chess::SIDE non_moving_side = side_on_move() == chess::WHITE ? chess::BLACK : chess::WHITE;
	const auto sq = * _pieces.at(moving_king).begin();
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

bool CBoard::is_checkmate() const 
{
	return is_check(); 
};

void CBoard::try_add_move(std::vector<CMove> & v, CMove mv)
{
	CBoard b(*this);
	const auto moving_side = _side;
	const auto non_moving_side = _side == chess::WHITE ? chess::BLACK : chess::WHITE;
	auto mem = b.make_move(mv);
	const CPiece moving_king(moving_side, chess::KING);
	const auto sq = * b._pieces[moving_king].begin();
	if (b.is_square_attacked(non_moving_side, sq))
	{
		//nowt
	}
	else
	{
		v.push_back(mv);
	}
}

std::vector<CMove> CBoard::legal_moves() 
{
	std::vector<CMove> v;
	v.reserve(20);
	const chess::SIDE other_side = (_side == chess::WHITE ? chess::BLACK : chess::WHITE);

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
							const CPiece atSquare = _board[to];

							if (atSquare.side() != chess::NONE)
							{
								// poss capture
								if (atSquare.side() == other_side)
								{
									try_add_move(v, CMove(index(from), index(to), CMove::MOVE_CAPTURE));
								}
								break;
							}
							try_add_move(v, CMove(index(from), index(to), CMove::MOVE_NONE));
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

					{
						const INT_SQUARES new_to = INT_SQUARES(from + direction);
						if (! off_board(new_to) && !is_occupied(new_to))
						{
							try_add_move(v, CMove(index(from), index(new_to), CMove::MOVE_NONE));

							// If on home rank we can move 2 squares as well, provided not blocked
							if ((from >> 4) == homeRank)
							{
								const INT_SQUARES new_to = INT_SQUARES(from + 2*direction);
								if (!is_occupied(new_to))
								{
									try_add_move(v, CMove(index(from), index(new_to), CMove::MOVE_DBL_PUSH));
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
								if (atSquare.side() != chess::NONE && atSquare.side() != _side)
								{
									try_add_move(v, CMove(index(from), index(new_to), CMove::MOVE_NONE));
								}
								else
								{
									// maybe EP
									if (_en_passant_square && new_to == *_en_passant_square)
									{
										try_add_move(v, CMove(index(from), index(new_to), CMove::FLAGS(CMove::MOVE_EN_PASSANT | CMove::MOVE_CAPTURE)));
									}
								}
							}
						}
					}


					// TODO promotions
					// TODO en passant

				} // end piece loop

			} // end pawn case
			break;
		}
	}

	// TODO move ordering

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
}

void CBoard::set_fen_position(std::string fen)
{

	if (fen == "1")
	{
		set_fen_position("2bqkbn1/2pppp2/np2N3/r3P1p1/p2N2B1/5Q2/PPPPKPP1/RNB2r2 w KQkq - 0 1");
		return;
	}

	clear_board();

	boost::tokenizer<> toks(fen);

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
				_board[row * 16 + col] = CPiece::from_char(c);
				col++;
			}
		}
		tok++;
	}

	if (tok.at_end())
		return;

	// side on move

	if (*tok == "b")
		set_side_on_move(chess::BLACK);
	else
		set_side_on_move(chess::WHITE);

	tok++;
	if (tok.at_end())
		return;

	// castling rights

	chess::CASTLING_RIGHTS cr = chess::CR_NONE;
	for (char c : *tok)
	{
		switch (c)
		{
		case 'K': cr = cr | chess::CR_WK; break;
		case 'Q': cr = cr | chess::CR_WQ; break;
		case 'k': cr = cr | chess::CR_BK; break;
		case 'q': cr = cr | chess::CR_BQ; break;
		}
	}

	tok++;
	if (tok.at_end())
		return;

	// ep
	if (*tok != "-")
	{
		
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
		if (_castling | chess::CR_WK) ss << "K";
		if (_castling | chess::CR_WQ) ss << "Q";
		if (_castling | chess::CR_BK) ss << "k";
		if (_castling | chess::CR_BQ) ss << "q";
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
		ss << std::endl;
	}
	return ss.str();
};

long CBoard::hash() const 
{
	return 0;
};
