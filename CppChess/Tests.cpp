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

namespace
{
	void MoveGenTests()
	{
		{
			CBoard b;
			b.set_fen_position("8/8/8/8/8/8/5R2/2k2QK1 b - - 0 1");
			cout << b.board();

			CHECK("checkmate is recognised", b.is_checkmate());
			CHECK_LT("the position evaluation is good", CEngine::heuristic(b), -9999);

		}
		{
			CBoard b;
			b.set_fen_position("8/8/8/8/8/7Q/5R2/2k3K1 b - - 0 1");
			cout << b.board();

			CHECK_LT("white is winning", CEngine::heuristic(b), -1000);
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

