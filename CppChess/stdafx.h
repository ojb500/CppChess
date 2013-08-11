// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>



// TODO: reference additional headers your program requires here

#include <algorithm>
#include <numeric>
#include <set>
#include <map>
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <assert.h>
#include <vector>
#include <sstream>
#include <functional>
#include <chrono>
#include <thread>

#include <boost\tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/range.hpp>

#ifdef _DEBUG
#define ASSERT(x) assert((x))
#else
#define ASSERT(x) ((void)0)	
#endif
