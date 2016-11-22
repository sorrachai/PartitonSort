#include "SaxPac.h"
#include "IndependentSetRectangles.h"

#include <algorithm>
#include <unordered_map>

namespace sp {
	using namespace std;

	bool AreIntervalsDistinct(const array<unsigned int, 2>& v1, const array<unsigned int, 2>& v2) {
		return v1[LowDim] > v2[HighDim] || v1[HighDim] < v2[LowDim];
	}

	bool IsOrderIndependent(const vector<Rule*> &rules, const vector<int> &fields) {
		for (size_t i = 0; i < rules.size(); i++) {
			for (size_t j = i + 1; j < rules.size(); j++) {
				if (!all_of(fields.begin(), fields.end(), [&](int d) {
					return AreIntervalsDistinct(rules[i]->range[d], rules[j]->range[d]);
				})) {
					return false;
				}
			}
		}
		return true;
	}

	/*
	Determines which pairs of rules each field can separate
	 */
	void FieldSeparation(const vector<Rule> rules, int field, unordered_map<int, vector<int>>& result) {
		for (size_t i = 0; i < rules.size(); i++) {
			result[i] = vector<int>();
			for (size_t j = 0; j < rules.size(); j++) {
				if (AreIntervalsDistinct(rules[i].range[field], rules[j].range[field])) {
					result[i].push_back(j);
					//printf("pair: [%u %u] [%u %u]\n", rules[i].range[field][LowDim], rules[i].range[field][HighDim], rules[j].range[field][LowDim], rules[j].range[field][HighDim]);
				}
			}
		}
	}

	GridOfTries AllFieldSetCover(const vector<Rule> &rules, vector<Rule>& remainder) {
		//printf("Inside\n");
		vector<vector<bool>> incompatable(rules.size(), vector<bool>(rules.size()));
		for (size_t i = 0; i < rules.size(); i++) {
			for (size_t j = 0; j < rules.size(); j++) {
				bool isAllowed = false;
				for (int f = 0; f < rules[i].dim; f++) {
					if (AreIntervalsDistinct(rules[i].range[f], rules[j].range[f])) {
						isAllowed = true;
						break;
					}
				}
				incompatable[i][j] = !isAllowed && (i!= j);
			}
		}
		//printf("Graph made\n");

		vector<int> indices;
		FindMIS(incompatable, indices);
		//printf("MIS: %u\n", indices.size());

		sort(indices.begin(), indices.end());
		
		vector<Rule> rl;
		size_t j = 0;
		for (size_t i = 0; i < rules.size(); i++) {
			if (j < indices.size() && indices[j] == i) {
				rl.push_back(rules[i]);
				j++;
			} else {
				remainder.push_back(rules[i]);
			}
		}
		if (rl.size() != indices.size()) {
			printf("Size problems!\n");
			printf("Indices\n");
			for (int i : indices) {
				printf("%i\n", i);
			}
			//printf("RL\n");
			//for (int i : rl) {
			//	printf("%i\n", i);
			//}
			printf("%lu %lu\n", rl.size(), indices.size());
			exit(0);
		}
		//printf("Out\n");

		GridOfTries got(rl);
		return got;
	}

	/*
	Makes a trie representing one SaxPac set
	*/
	GridOfTries GreedySetCover(const vector<Rule> &rules, const vector<int> &fields, vector<Rule> & remainder, int fieldLimit, bool useSpecial) {
		//printf("Field Limit: %d\n", fieldLimit);

		vector<unordered_map<int, vector<int>>> sfSeparations;
		for (int f : fields) {
			unordered_map<int, vector<int>> s;
			FieldSeparation(rules, f, s);
			sfSeparations.push_back(s);
		}

		// Make a 2D graph showing which rules cannot be with other rules 
		vector<vector<bool>> unselected(rules.size(), vector<bool>(rules.size(), true));
		// A rule can always be with itself
		for (size_t i = 0; i < rules.size(); i++) {
			unselected[i][i] = false;
		}
		vector<int> fl; // Fields used
		while (fieldLimit > 0) {
			int bestValue = 0;
			size_t best = 0;
			for (size_t i = 0; i < fields.size(); i++) {
				int value = 0;
				for (auto m : sfSeparations[i]) {
					for (int j : m.second) {
						if (unselected[m.first][j]) {
							value++;
						}
					}

				}
				if (value > bestValue) {
					bestValue = value;
					best = i;
				}
			}
			for (auto m : sfSeparations[best]) {
				for (int j : m.second) {
					unselected[m.first][j] = false;
				}
			}

			fl.push_back(best);
			fieldLimit--;
		}

		//vector<int> indices;
		//FindMIS(unselected, indices);
		//sort(indices.begin(), indices.end());
		//vector<Rule> rl;
		//size_t j = 0;
		//for (size_t i = 0; i < rules.size(); i++) {
		//	if (indices[j] == i) {
		//		rl.push_back(rules[i]);
		//		j++;
		//	} else {
		//		remainder.push_back(rules[i]);
		//	}
		//}

		//fl.assign({ 0, 1 });
		
		//printf("Fields: %d %d\n", fl[0], fl[1]);

		vector<Rule> rl;
		if (useSpecial && fl.size() == 2) {
			//printf("Use special\n");
			Projection2D proj(fl[0], fl[1]);
			proj.FindMIS(rules, rl, remainder);
			
		} else {
			//printf("Use MIS-Alt: %u\n", fl.size());
			vector<int> indices;
			FindMISAlt(unselected, indices);

			int i = 0;
			for (size_t j = 0; j < rules.size(); j++) {
				if (j == indices[i]) {
					rl.push_back(rules[j]);
					i++;
				} else {
					remainder.push_back(rules[i]);
				}
			}
		}

		//printf("Started: %u, mis: %u, left: %u\n", rules.size(), rl.size(), remainder.size());

		GridOfTries got(rl); //TODO: use selected elements
		return got;
	}

	/*void SingleFieldMIS(const vector<Rule*> &rules, vector<int> indices, int field) {

	}*/

	//GridOfTries GreedySetCover(const vector<Rule*> &rules, const vector<int>& fields, vector<Rule*>& remainder, int fieldLimit) {
	//	vector<vector<int>> sfMises;
	//	for (int f : fields) {
	//		vector<int> r;
	//		SingleFieldMIS(rules, r, f);
	//		sfMises.push_back(r);
	//	}

	//	int numUnselected = rules.size();
	//	vector<bool> unselected(rules.size(), true);
	//	vector<int> resultFields;
	//	while (fieldLimit && numUnselected) {
	//		int bestValue = 0;
	//		size_t best = 0;
	//		for (size_t i = 0; i < fields.size(); i++) {
	//			int value = 0;
	//			for (int index : sfMises[i]) {
	//				if (unselected[index]) {
	//					value++;
	//				}
	//			}
	//			if (value > bestValue) {
	//				bestValue = value;
	//				best = i;
	//			}
	//		}
	//		numUnselected -= bestValue;
	//		for (int index : sfMises[best]) {
	//			unselected[index] = false;
	//		}
	//		resultFields.push_back(best);
	//	}
	//	vector<Rule*> result;
	//	for (size_t i = 0; i < unselected.size(); i++) {
	//		if (unselected[i]) {
	//			remainder.push_back(rules[i]);
	//		} else {
	//			result.push_back(rules[i]);
	//		}
	//	}
	//	fieldLimit--;
	//	return; // TODO : grid of trie
	//}

	SaxPac::SaxPac(const unordered_map<string, string>& args) : 
		fieldLimit(GetIntOrElse(args, "SaxPac.Fields", 2)),
		useSpecial(GetBoolOrElse(args, "SaxPac.Special", true))
		{}


	SaxPac::~SaxPac() {}

	void SaxPac::ConstructClassifier(const std::vector<Rule>& rules) {
		//fieldLimit = rules[0].dim;

		this->rules = rules;
		vector<Rule> rl = rules;
		vector<int> fields = { 0, 1, 2, 3, 4 }; // TODO : from rules

		while (!rl.empty()) {
			vector<Rule> remain;
			GridOfTries got = GreedySetCover(rl, fields, remain, fieldLimit, useSpecial);
			//GridOfTries got = AllFieldSetCover(rl, remain);
			//printf("Trie Size: %d rules\n", got.CountRules());
			tries.push_back(got);
			rl = remain;
			numTables++;
		}
		printf("Test only number of partitions (Tables); No actual classifier implemented\n"); 
	}

	int SaxPac::ClassifyAPacket(const Packet& one_packet) {
		// TODO : something with GoTs
		return -1;
	}

	void SaxPac::DeleteRule(size_t index) {

	}

	void SaxPac::InsertRule(const Rule& one_rule) {

	}

	Memory SaxPac::MemSizeBytes() const {
		return 0; // TODO
	}

	int SaxPac::MemoryAccess() const {
		return 0; // TODO
	}
}
