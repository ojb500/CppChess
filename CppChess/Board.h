#pragma once
#include "Piece.h"
#include "Square.h"
#include "Move.h"
#include <boost/optional.hpp>

class CBoardInterface
{
public:
	virtual ~CBoardInterface() {};

	virtual void add_piece(CPiece p, chess::SQUARES sq) = 0;
	//	virtual CMemento make_move(CMove mv) = 0;
	//	virtual void unmake_move(CMemento m) = 0;

	virtual bool is_check() const = 0;
	virtual bool is_checkmate() = 0;

	virtual chess::SIDE side_on_move() const = 0;

	virtual std::vector<CMove> legal_moves() = 0;
	virtual CPiece piece_at_square(chess::SQUARES sq) const = 0;
	virtual std::string san_name(CMove m) const = 0;

	virtual std::string fen() const = 0;

	virtual long hash() const = 0;
};

class CBoard: public CBoardInterface
{
public:
	enum INT_SQUARES
	{
		A8, B8, C8, D8, E8, F8, G8, H8, XA8, XB8, XC8, XD8, XE8, XF8, XG8, XH8,
		A7, B7, C7, D7, E7, F7, G7, H7,	XA7, XB7, XC7, XD7, XE7, XF7, XG7, XH7,
		A6, B6, C6, D6, E6, F6, G6, H6,	XA6, XB6, XC6, XD6, XE6, XF6, XG6, XH6,
		A5, B5, C5, D5, E5, F5, G5, H5,	XA5, XB5, XC5, XD5, XE5, XF5, XG5, XH5,
		A4, B4, C4, D4, E4, F4, G4, H4,	XA4, XB4, XC4, XD4, XE4, XF4, XG4, XH4,
		A3, B3, C3, D3, E3, F3, G3, H3,	XA3, XB3, XC3, XD3, XE3, XF3, XG3, XH3,
		A2, B2, C2, D2, E2, F2, G2, H2,	XA2, XB2, XC2, XD2, XE2, XF2, XG2, XH2,
		A1, B1, C1, D1, E1, F1, G1, H1,	XA1, XB1, XC1, XD1, XE1, XF1, XG1, XH1,
		INT_SQUARE_FIRST = A8, INT_SQUARE_LAST = XH1
	};

	typedef std::map<CPiece, std::set<INT_SQUARES>, less_piece> PieceTable;

	struct CMemento
	{
	public:
		CMemento(CMove mv) : move(mv), cr(chess::CR_ALL), ep() {};

		CPiece captured;
		chess::CASTLING_RIGHTS cr;
		boost::optional<CBoard::INT_SQUARES> ep;
		CMove move;
	};

	virtual void add_piece(CPiece p, chess::SQUARES sq) override;
	CMemento make_move(CMove mv);
	void unmake_move(CMemento m);

	virtual bool is_check() const override;
	virtual bool is_checkmate() override;
	virtual chess::SIDE side_on_move() const override;
	void set_side_on_move(chess::SIDE side);
	virtual std::vector<CMove> legal_moves() override;
	virtual CPiece piece_at_square(chess::SQUARES sq) const override;
	virtual std::string san_name(CMove m) const override;
	
	int ply() const;

	std::string board() const;

	virtual std::string fen() const override;
	void set_fen_position(std::string);

	virtual long hash() const override;

	CBoard();

	const PieceTable& piece_table() const;

private:
	CPiece _board[128];
	PieceTable _pieces;

	bool is_occupied(INT_SQUARES sq) const;
	typedef std::map<CPiece, std::map<INT_SQUARES, std::vector<std::vector<INT_SQUARES>>>, less_piece> LookupTables;

	bool is_square_attacked(chess::SIDE attacker, INT_SQUARES sq) const;

	void try_add_move(std::vector<CMove> & v, CMove mv);
	void try_add_castling_move(std::vector<CMove> & v, CMove mv);

	void clear_board();

	static LookupTables createLookupTables();
	static void AddOffsetAttacks(INT_SQUARES start, int offset, int max, std::vector<std::vector<INT_SQUARES>> & lt);
	static LookupTables _lookups;


	chess::SIDE _side;
	chess::CASTLING_RIGHTS _castling;

	boost::optional<INT_SQUARES> _en_passant_square;

	int _halfmoves;
	int _fullmove;

	friend class CTests;
};