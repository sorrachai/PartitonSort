#pragma once

#include "../ElementaryClasses.h"

namespace sp {
	class GridOfTries {
	public:
		GridOfTries(const std::vector<Rule>& rules);
		~GridOfTries();

		int CountRules() const { return numRules; }

	private:
		int numRules;
	};
}



