#pragma once
#include "Piece.h"
#include "Square.h"
#include "Move.h"
#include <boost/optional.hpp>
#include "Zobrist.h"

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
	virtual std::vector<CMove> legal_moves_q() = 0;
	virtual CPiece piece_at_square(chess::SQUARES sq) const = 0;
	virtual std::string san_name(CMove m) const = 0;

	virtual std::string fen() const = 0;

	virtual CZobrist hash() const = 0;
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


	struct CMemento
	{
	public:
		CMemento(CMove mv) : move(mv), cr(chess::CR_ALL), ep(), hash(0) {};

		CZobrist hash;
		CPiece captured;
		chess::CASTLING_RIGHTS cr;
		CBoard::INT_SQUARES ep;
		CMove move;
	};

	class CPieceList : public std::set<INT_SQUARES>
	{
	public:
		const INT_SQUARES only()const
		{
			ASSERT(size() == 1);
			return *cbegin();
		};
	};

	class CPieceTable 
	{

	private:
		typedef std::vector<CPieceList> Array;
		Array _list;

	public:
		CPieceTable()
			: _list(12)
		{

		}
		const CPieceList& at(const CPiece p)const
		{
			const int i = (p.piece() - 1) + (p.side() == chess::BLACK ? 6 : 0);
			return _list[i];
		}
		CPieceList& operator[](const CPiece p)
		{
			const int i = (p.piece() - 1) + (p.side() == chess::BLACK ? 6 : 0);
			return _list[i];
		}
		const CPieceList& operator[](const CPiece p)const
		{
			const int i = (p.piece() - 1) + (p.side() == chess::BLACK ? 6 : 0);
			return _list[i];
		}
		void clear()
		{
			for (auto&a : _list)
			{
				a.clear();
			}
		}
		bool equals(const CPieceTable& other)const
		{
			for (int side=0; side<2; ++side)
				for (int i=1;i<7;++i)
				{
					const CPiece pc(static_cast<chess::SIDE>(side), static_cast<chess::PIECE>(i));
					if (at(pc) != other[pc])
						return false;
				}
				return true;
		}
	};

	typedef CPieceTable PieceTable;

	virtual void add_piece(CPiece p, chess::SQUARES sq) override;
	CMemento make_move(CMove mv);
	void unmake_move(CMemento m);

	virtual bool is_check() const override;
	virtual bool is_checkmate() override;
	virtual chess::SIDE side_on_move() const override;
	void set_side_on_move(chess::SIDE side);
	virtual std::vector<CMove> legal_moves() override;
	virtual std::vector<CMove> legal_moves_q() override;
	virtual CPiece piece_at_square(chess::SQUARES sq) const override;
	CPiece piece_at_square(INT_SQUARES sq) const;
	virtual std::string san_name(CMove m) const override;

	int ply() const;

	std::string board() const;

	virtual std::string fen() const override;

	bool assert_piecelist_consistent() const;

	void set_fen_position(std::string);

	virtual CZobrist hash() const override;

	CBoard();

	const PieceTable& piece_table() const;

	static CBoard::INT_SQUARES int_index(chess::SQUARES sq)
	{
		return CBoard::INT_SQUARES(sq + (sq & ~7));
	}

	static chess::SQUARES index(CBoard::INT_SQUARES sq)
	{
		const int row = (sq / 16);
		const int col = (sq % 16);
		ASSERT(col < 8);
		return chess::SQUARES((row * 8) + col);
	}

	static bool off_board(CBoard::INT_SQUARES sq)
	{
		return (sq & 0x88) != 0;
	}

	bool is_square_attacked(chess::SIDE attacker, INT_SQUARES sq) const;
	CBoard::INT_SQUARES get_smallest_attacker(chess::SIDE attacker, INT_SQUARES sq) const;

private:
	CPiece _board[128];
	PieceTable _pieces;

	bool is_occupied(INT_SQUARES sq) const;
	typedef std::map<short, std::vector<std::vector<std::vector<INT_SQUARES>>>> LookupTables;

	CZobrist hash_from_scratch() const;

	void try_add_move(CBoard& b, std::vector<CMove> & v, CMove mv);
	void try_add_castling_move(CBoard& b, std::vector<CMove> & v, CMove mv);

	void clear_board();

	static LookupTables createLookupTables();
	static void AddOffsetAttacks(INT_SQUARES start, int offset, int max, std::vector<std::vector<INT_SQUARES>> & lt);
	static LookupTables _lookups;


	chess::SIDE _side;
	chess::CASTLING_RIGHTS _castling;

	INT_SQUARES _en_passant_square;

	int _halfmoves;
	int _fullmove;

	friend class CTests;
	friend class CBoardMutator;

	CZobrist _hash;
};