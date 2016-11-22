 #pragma once


#include "../ElementaryClasses.h"

#include <vector>

namespace tcam {
	unsigned int SizeAsPrefixes(const std::array<unsigned int, 2>& range);
	unsigned int NumOfPrefixRules(const Rule& r);
	unsigned int SizeAsPrefixRules(const std::vector<Rule>& rules);
}
