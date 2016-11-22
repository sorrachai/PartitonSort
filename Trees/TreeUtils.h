#pragma once

#ifndef TREE_UTILS_H
#define TREE_UTILS_H

#include "../ElementaryClasses.h"

struct Range {
	Range(Point l, Point h) : low(l), high(h) {};
	Range() : low(0), high(0) {};

	Point low;
	Point high;

	inline bool ContainsPoint(Point x) {
		return low <= x && x <= high;
	}
};

namespace TreeUtils {
	inline Range V2R(const std::array<unsigned int, 2> & v) {
		return Range{ v[LowDim], v[HighDim] };
	}

	Range RangeInBoundary1D(Range r, Range boundary);

	bool CompareRanges(const Range& a, const Range& b);

	bool InsideOutCompare(const Range& a, const Range& b);

	bool CompareSecond(const std::pair<int, int> & x, const std::pair<int, int> & y);

	bool AreEqual(const Rule& rule1, const Rule& rule2, const std::vector<Range>& boundary);

	bool AreSameRules(const std::list<Rule*>& c1, const std::list<Rule*>& c2);

	void RemoveRedund(std::list<Rule*> & rules, const std::vector<Range>& boundary);

	inline bool ContainsRange(const Range& outer, const Range& inner) {
		return outer.low <= inner.low && outer.high >= inner.high;
	}

	struct RangeComp {
		bool operator()(const Range& lhs, const Range& rhs) const { return InsideOutCompare(lhs, rhs); }
	};
}

#endif
