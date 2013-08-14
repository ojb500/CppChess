#include "stdafx.h"
#include "Perft.h"
#include <iostream>
using namespace std;

CPerft::CPerft(CBoard b)
	: b(b)
{

}

CPerft::DivideResultMap CPerft::Divide(int depth)
{
	DivideResultMap div;
	const auto & moves = b.legal_moves();
	cout << moves.size() << " moves" << endl;
	uint64_t total_nodes = 0;
	for (const auto & move : moves)
	{
		cout << "  " << left << setw(8) << move.longer_algebraic();
		CBoard b2(b);
		b2.make_move(move);
		CPerft p2(b2);
		auto pr = p2.DoPerft(depth-1);
		cout << right << setw(15) << pr.nodes << endl;
		total_nodes += pr.nodes;
	}
	cout << "total nodes " << total_nodes << endl;
	return div;
}

CPerft::SPerftResult CPerft::DoPerft(int depth)
{
	SPerftResult res;
	const auto & moves = b.legal_moves();
	if (depth == 1)
	{
		res.nodes = moves.size();
		return res;
	}

	for (const auto & move : moves)
	{
		{
			// test flags
			if (move.is_capture())
				res.captures++;
			if (move.is_check())
				res.checks++;
			if (move.is_promotion())
				res.promotions++;
			if (move.is_en_passant_capture())
				res.ep++;
			if (move.is_castle())
				res.castles++;
		}
		auto mem = b.make_move(move);
		res += DoPerft(depth-1);
		b.unmake_move(mem);
	}
	return res;
}

CPerft::~CPerft(void)
{
}
