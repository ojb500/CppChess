#ifdef _DEBUG

#define MUTATOR_CHECK_FENS
#undef MUTATOR_CHECK_HASHES
#undef MUTATOR_CHECK_PIECELIST

#else

#undef MUTATOR_CHECK_FENS
#undef MUTATOR_CHECK_HASHES
#undef MUTATOR_CHECK_PIECELIST

#endif