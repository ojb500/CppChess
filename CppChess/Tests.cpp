#include "stdafx.h"
#include "Tests.h"
#include "Engine.h"
#include "Board.h"
using namespace std;

#define CHECK(msg,cond) do \
{ \
	cout << "checking " << msg << "..."; \
	if (!(cond)) \
{ \
	cout << "FAILED " << endl; \
	ASSERT(false); \
} \
		else { \
		cout << "ok!" << endl; \
} \
} while(0)  

#define CHECK2(msg,msg2,cond) do \
{ \
	cout << "checking " << msg << "..."; \
	if (!(cond)) \
{ \
	cout << "FAILED (" << msg2 << ")" << endl; \
	ASSERT(false); \
} \
		else { \
		cout << "ok!" << endl; \
} \
} while(0)  

#define CHECK3(msg,msgfail,msgok,cond) do \
{ \
	cout << "checking " << msg << "..."; \
	if (!(cond)) \
{ \
	cout << "FAILED (" << msgfail << ")" << endl; \
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

				const std::string fen_after = "k7/8/8/8/8/4p3/8/7K w - - 1 2";
				CHECK_EQ("move made correctly", b.fen(), fen_after);
				
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
			CHECK_LT("the position evaluation is good", CEngine::heuristic(b), -9999);

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

			CHECK_GT("white is winning", CEngine::heuristic(b), 1000);
			// CHECK_LT("mate in two is recognisd", CEngine::heuristic(b), -99998);
		}
		{
			CUciSession s(cin, cout, cout);

		}
	}
}

CTests::CTests(void)
{
	MoveGenTests();
}

