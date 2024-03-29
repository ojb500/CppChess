#pragma once

namespace chess
{
	enum CASTLING_RIGHTS
	{
		CR_NONE = 0,
		CR_WK = 1,
		CR_WQ = 2,
		CR_BK = 4,
		CR_BQ = 8,
		CR_ALL = 15,
	};

	inline CASTLING_RIGHTS operator|(CASTLING_RIGHTS a, CASTLING_RIGHTS b)
	{return static_cast<CASTLING_RIGHTS>(static_cast<int>(a) | static_cast<int>(b));}

	inline CASTLING_RIGHTS operator&(CASTLING_RIGHTS a, CASTLING_RIGHTS b)
	{return static_cast<CASTLING_RIGHTS>(static_cast<int>(a) & static_cast<int>(b));}

	enum SQUARES
	{
		A8, B8, C8, D8, E8, F8, G8, H8,
		A7, B7, C7, D7, E7, F7, G7, H7,
		A6, B6, C6, D6, E6, F6, G6, H6,
		A5, B5, C5, D5, E5, F5, G5, H5,
		A4, B4, C4, D4, E4, F4, G4, H4,
		A3, B3, C3, D3, E3, F3, G3, H3,
		A2, B2, C2, D2, E2, F2, G2, H2,
		A1, B1, C1, D1, E1, F1, G1, H1,
		SQUARE_FIRST = A8, SQUARE_LAST = H1,
	};

	const std::string SQUARE_STRINGS[] = {
		"a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
		"a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
		"a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
		"a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
		"a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
		"a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
		"a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
		"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
	};

	inline SQUARES square_from_string(std::string s)
	{
		ASSERT(s.length() == 2);
		char file = s.at(0);
		char rank = s.at(1);
		int col = (file - 'a');
		int row = 7 - (rank - '1');
		return SQUARES(row * 8 + col);
	};

	inline std::string name_of_square(chess::SQUARES s)
	{
		return SQUARE_STRINGS[s];
	};

	inline int square_file(chess::SQUARES s)
	{
		return (s % 8);
	}

	inline char square_file_char(chess::SQUARES s)
	{
		return square_file(s) + 'a';
	}

	inline int square_rank(chess::SQUARES s)
	{
		return 7 - (s / 8);
	}

	inline char square_rank_char(chess::SQUARES s)
	{
		return square_rank(s) + '1';
	}
};