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
            cout << "FAILED" << endl; \
			ASSERT(false); \
        } \
		else { \
			cout << "ok!" << endl; \
		} \
    } while(0)  


namespace
{
	void MoveGenTests()
	{
		{
			CBoard b;
			b.set_fen_position("8/8/8/8/8/8/5R2/2k2QK1 b - - 0 1");
			CHECK("checkmate is recognised", b.is_checkmate());
		}
	}
}

CTests::CTests(void)
{
	MoveGenTests();
}

