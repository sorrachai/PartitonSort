 #pragma once

#include "../ElementaryClasses.h"

#include <vector>

typedef std::array<Point, 2> GRange;

class EffectiveGrid {
public:
	EffectiveGrid(const std::vector<Rule>& rules, int dim);

	size_t SizeOfRule(const Rule& r) const;
	size_t SizeOfRuleList(const std::vector<Rule>& rules) const;

	void SplitRule(const Rule& r, std::vector<Rule>& receivingList) const;
	void SplitRuleList(const std::vector<Rule>& rules, std::vector<Rule>& receivingList) const;
	
	static size_t GridSize(const std::vector<Rule>& rules, int numDims);
	static size_t GridVolume(const std::vector<Rule>& rules, int numDims);
	static void GridSplit(const std::vector<Rule>& rules, int numDims, std::vector<Rule>& receivingList);

	static void SortableSplit(const std::vector<Rule>& rules, const std::vector<int>& dimOrder, std::vector<Rule>& receivingList, size_t dimIndex = 0);
private:
	int dim;
	std::vector<GRange> ranges;

	size_t BinarySearch(size_t lowIndex, size_t highIndex, unsigned int pt) const;

	static void RemoveConcealedRules(std::vector<Rule>& rules);
};

