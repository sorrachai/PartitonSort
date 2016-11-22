#include "Tcam.h"

#include <algorithm>

using namespace std;

namespace tcam {
	unsigned int PrefixSizeHelper(unsigned int low, unsigned int high, unsigned int boundLow, unsigned int boundHigh) {
		if (boundHigh < low || boundLow > high) return 0;
		if (low <= boundLow && high >= boundHigh) return 1;

		unsigned int boundMid = (boundLow / 2) + (boundHigh / 2); // Guard against overflow
		return PrefixSizeHelper(low, high, boundLow, boundMid) + PrefixSizeHelper(low, high, boundMid + 1, boundHigh);
	}

	unsigned int SizeAsPrefixes(const array<unsigned int, 2>& range) {
		unsigned int low = range[LowDim];
		unsigned int high = range[HighDim];
		return PrefixSizeHelper(low, high, 0, 0xFFFFFFFF);
	}

	unsigned int NumOfPrefixRules(const Rule& r) {
		unsigned int area = 1;
		for (const auto& range : r.range) {
			area *= SizeAsPrefixes(range);
		}
		return area;
	}
	unsigned int SizeAsPrefixRules(const vector<Rule>& rules) {
		unsigned int sum = 0;
		for (const Rule& r : rules) {
			sum += NumOfPrefixRules(r);
		}
		return sum;
	}
}
