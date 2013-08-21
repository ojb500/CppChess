#include <exception>

#ifdef _DEBUG

#undef MUTATOR_CHECK_FENS
#define MUTATOR_CHECK_HASHES
#undef MUTATOR_CHECK_PIECELIST

#else

#undef MUTATOR_CHECK_FENS
#undef MUTATOR_CHECK_HASHES
#undef MUTATOR_CHECK_PIECELIST

#endif

#ifdef _DEBUG
#define RELEASE_ASSERT(cond) ASSERT(cond)
#else
#define RELEASE_ASSERT(cond) do { \
	if (!(cond)) \
	{ \
	throw std::runtime_error("Assertion failed"); \
	}} while (0)
#endif