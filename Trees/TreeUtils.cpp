#include "TreeUtils.h"

#include <algorithm>

using namespace std;

namespace TreeUtils {

	Range RangeInBoundary1D(Range r, Range boundary) {
		uint32_t low, high;
		low = max(r.low, boundary.low);
		high = min(r.high, boundary.high);
		return Range{ low, high };
	}

	bool CompareRanges(const Range& a, const Range& b) {
		if (a.low != b.low) {
			return a.low < b.low;
		} else {
			return a.high < b.high;
		}
	}

	bool InsideOutCompare(const Range& a, const Range& b) {
		if (ContainsRange(a, b)) {
			return false;
		} else if (ContainsRange(b, a)) {
			return true;
		} else {
			return CompareRanges(a, b);
		}
	}

	bool CompareSecond(const pair<int, int> & x, const pair<int, int> & y) {
		return x.second > y.second;
	}

	bool AreEqual(const Rule& rule1, const Rule& rule2, const vector<Range>& boundary) {
		for (size_t i = 0; i < boundary.size(); i++) {
			Range r1 = RangeInBoundary1D(V2R(rule1.range[i]), boundary[i]);
			Range r2 = RangeInBoundary1D(V2R(rule2.range[i]), boundary[i]);
			if (r1.low > r2.low || r1.high < r2.high) {
				return false;
			}
		}
		return true;
	}

	bool AreSameRules(const list<Rule*>& c1, const list<Rule*>& c2) {
		if (c1.empty() || c2.empty()) {
			return false;
		}
		if (c1.size() != c2.size()) {
			return false;
		}


		size_t num = 0;
		for (const Rule* r1 : c1) {
			bool found = false;
			for (const Rule* r2 : c2) {
				if (r1->priority == r2->priority) {
					found = true;
					num++;
					break;
				}
			}
			if (!found) {
				return false;
			}
		}
		if (num != c1.size()) {
			printf("ERR: found the wrong number of matches.\n");
		}
		return true;
	}

	void RemoveRedund(list<Rule*> & rules, const vector<Range>& boundary) {
		list<Rule*> rulelist;
		for (Rule* rule : rules) {
			bool found = false;
			for (Rule* mule : rulelist) {
				if (AreEqual(*mule, *rule, boundary)) {
					found = true;
					break;
				}
			}
			if (!found) {
				rulelist.push_back(rule);
			}
		}
		rules.clear();
		rules = rulelist;
		rules.unique([](Rule* r1, Rule* r2) { return r1->priority == r2->priority; });
	}
}
