#include "stdafx.h"
#include "Tests.h"
#include "Engine.h"
#include "Heuristic.h"
#include "Board.h"
using namespace std;

int s_testsRun = 0;
int s_testsFailed = 0;

#define CHECK(msg,cond) do \
{ \
	cout << "checking " << msg << "..."; \
	s_testsRun++; \
	if (!(cond)) \
{ \
	cout << "FAILED " << endl; \
	s_testsFailed++; \
	ASSERT(false); \
} \
		else { \
		cout << "ok!" << endl; \
} \
} while(0)  

#define CHECK2(msg,msg2,cond) do \
{ \
	cout << "checking " << msg << "..."; \
	s_testsRun++; \
	if (!(cond)) \
{ \
	cout << "FAILED (" << msg2 << ")" << endl; \
	s_testsFailed++; \
	ASSERT(false); \
} \
		else { \
		cout << "ok!" << endl; \
} \
} while(0)  

#define CHECK3(msg,msgfail,msgok,cond) do \
{ \
	cout << "checking " << msg << "..."; \
	s_testsRun++; \
	if (!(cond)) \
{ \
	cout << "FAILED (" << msgfail << ")" << endl; \
	s_testsFailed++; \
	ASSERT(false); \
} \
		else { \
		cout << "ok! (" << msgok << ")" << endl; \
} \
} while(0)

#define CHECK_GT(msg,val,val2) CHECK3(msg,val << " <= " << val2,val << " > " << val2,val > val2)
#define CHECK_LT(msg,val,val2) CHECK3(msg,val << " >= " << val2,val << " < " << val2, val < val2)
#define CHECK_EQ(msg,val,val2) CHECK3(msg,val << " != " << val2,val2, val == val2)

namespace
{
	void EngineTests()
	{
		{// easy mate in 1
			CBoard b;
			CUciSession s(cin, cout, cout);
			CEngine e(s);
			b.set_fen_position("8/8/8/8/8/8/6PP/2r1k2K b - - 0 1");
			e.set_position(b);
			auto think = e.Think();
			cout << think.long_algebraic() << endl;
			
		}
		{// win the queen
			CBoard b;
			CUciSession s(cin, cout, cout);
			CEngine e(s);
			b.set_fen_position("8/4Q3/8/8/4K3/8/1q6/5k2 b - - 0 1");
			e.set_position(b);
			auto think = e.Think();
			cout << think.long_algebraic() << endl;
			
		}
		{ // tricky mate
			CBoard b;
			CUciSession s(cin, cout, cout);
			CEngine e(s);
			b.set_fen_position("3r3k/2qnb2p/p2p4/1pnBprP1/1P3P2/P6R/2P1Q2P/R1B3K1 w - - 0 23");
			e.set_position(b);
			auto think = e.Think();
			cout << think.long_algebraic() << endl;
		}
		{
			CBoard b;
			CUciSession s(cin, cout, cout);
			CEngine e(s);
			b.set_fen_position("k7/6q1/8/8/3Q4/8/8/7K w - - 0 1");
			e.set_position(b);
			auto think = e.Think();
			cout << think.long_algebraic() << endl;
		}
	}
	void MoveGenTests()
	{

		{
			CBoard b;
			b.set_fen_position("k7/8/8/8/3pP3/8/8/7K b - e3 0 1");
			cout << b.board();

			const auto lm = b.legal_moves();
			for (const auto & mv : lm)
			{
				cout << mv.long_algebraic() << " ";
			}
			cout << endl;
			auto ep = find_if(lm.begin(), lm.end(), [&](const CMove & mv)
			{
				return mv.is_en_passant_capture();
			});

			CHECK("en passant move is found", ep != lm.end());

			std::string orig_fen = b.fen();

			if (ep != lm.end())
			{
				CHECK_EQ("it is d4e3", ep->long_algebraic(), "d4e3");


				auto mem = b.make_move(*ep);

				const std::string fen_after = "k7/8/8/8/8/4p3/8/7K w - - 0 2";
				CHECK_EQ("move made correctly", b.fen(), fen_after);
				cout << b.board() << endl;


				b.unmake_move(mem);

				cout << b.board() << endl;

				CHECK_EQ("move unmade correctly", b.fen(), orig_fen);

			}

		}
		{
			CBoard b;
			b.set_fen_position("8/8/8/8/8/8/5R2/2k2QK1 b - - 0 1");
			cout << b.board();

			CHECK("checkmate is recognised", b.is_checkmate());
			CHECK_LT("the position evaluation is good", CHeuristic(b).value(), -9999);

		}
		{
			CBoard b;
			b.set_fen_position("P3");
			cout << b.board();
			const auto brd = b.board();

			const auto lm = b.legal_moves();
			cout << b.board();
			CHECK_EQ("board is not mangled", b.board(), brd); 

			CHECK_EQ("correct # of legal moves", lm.size(), 14);
			// CHECK_LT("mate in two is recognisd", CEngine::heuristic(b), -99998);
		}
		{
			CBoard b;
			b.set_fen_position("8/8/8/8/8/7Q/5R2/2k3K1 w - - 0 1");
			cout << b.board();

			CHECK_GT("white is winning", CHeuristic(b).value(), 1000);
			// CHECK_LT("mate in two is recognisd", CEngine::heuristic(b), -99998);
		}
	}

	void MoveOrderTests()
	{
		{
			CBoard b;
			b.set_fen_position("r2q1rk1/pp2pppp/2p5/1n1p1b2/8/3Q4/PPP2PPP/RN3RK1 w - - 0 1");
			cout << b.board();

			CUciSession s(cin, cout, cout);
			CEngine e(s);
			e.set_position(b);
			auto think = e.Think();
			cout << think.long_algebraic() << endl;
		}
	}
}

CTests::CTests(void)
{
	//
	MoveGenTests();
	EngineTests();
	MoveOrderTests();
	

	cout << "ran " << s_testsRun << " tests of which " << s_testsFailed << " failed" << endl;
}

