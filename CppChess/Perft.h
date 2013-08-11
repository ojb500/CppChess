#pragma once
#include "Board.h"
class CPerft
{
private:
	CBoard b;
public:
	struct SPerftResult
	{
		uint64_t nodes;
		uint64_t ep;
		uint64_t captures;
		uint64_t promotions;
		uint64_t checks;
		uint64_t castles;

		SPerftResult() : nodes(0), ep(0), captures(0), promotions(0), checks(0), castles(0) {}
		SPerftResult& operator+= (const SPerftResult & other)
		{
			nodes += other.nodes;
			/*ep += other.ep;
			captures += other.captures;
			promotions += other.promotions;
			checks += other.checks;
			*/
			return *this;
		};
	};
	
	CPerft(CBoard b);
	~CPerft(void);
	typedef std::map<std::string, uint64_t> DivideResultMap;
	DivideResultMap Divide(int depth);

	SPerftResult DoPerft(int depth);

};

