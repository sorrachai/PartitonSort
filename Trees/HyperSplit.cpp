#include <iostream>
#include <set>
#include <algorithm>
#include <utility>

#include "HyperSplit.h"

#include <string>

#define NodeSize 8

using namespace std;

typedef pair<unsigned int, unsigned int> Range;

void CheckBounds(const vector<vector<unsigned int>> &bounds, const string &s) {
	if (bounds.size() != 5) {
		cout << s << endl;
		cout << "Incorrect bounds: " << bounds.size() << endl;
		throw "Bad size!";
	}
}

void CheckBounds(const vector<vector<unsigned int>> &bounds) {
	string s = "";
	CheckBounds(bounds, s);
}

void PrintBounds(const vector<vector<unsigned int>> &bounds) {
	cout << "(";
	for (const vector<unsigned int>& s : bounds) {
		cout << s[LowDim] << "-" << s[HighDim] << ",";
	}
	cout << ")" << endl;
}

vector<Rule> LeftList(const vector<Rule>& rules, int dim, unsigned int splitPoint) {
	vector<Rule> leftList;
	for (const Rule& r : rules) {
		if (r.range[dim][LowDim] <= splitPoint) {
			leftList.push_back(r);
		}
	}
	return leftList;
}

vector<Rule> RightList(const vector<Rule>& rules, int dim, unsigned int splitPoint) {
	vector<Rule> rightList;
	for (const Rule& r : rules) {
		if (r.range[dim][HighDim] > splitPoint) {
			rightList.push_back(r);
		}
	}
	return rightList;
}

vector<unsigned int> GetSplitPoints(const vector<vector<unsigned int>>& bounds, const vector<Rule>& rules, int dim) {
	set<unsigned int> lows, highs;
	lows.insert(bounds[dim][LowDim]);
	highs.insert(bounds[dim][HighDim]);

	for (const Rule& r : rules) {
		lows.insert(max(r.range[dim][LowDim], bounds[dim][LowDim]));
		highs.insert(min(r.range[dim][HighDim], bounds[dim][HighDim]));
	}
	lows.erase(lows.find(bounds[dim][LowDim]));
	for (unsigned int l : lows) {
		highs.insert(l - 1);
	}

	vector<unsigned int> result;
	for (unsigned int h : highs) {
		result.push_back(h);
	}
	sort(result.begin(), result.end());
	return result;
}

vector<Range> GetRanges(const vector<vector<unsigned int>>& bounds, const vector<Rule>& rules, int dim) {
	vector<unsigned int> splits = GetSplitPoints(bounds, rules, dim);
	vector<Range> results;
	unsigned int l = bounds[dim][LowDim];
	for (unsigned int r : splits) {
		results.push_back(Range(l, r));
		l = r + 1;
	}
	return results;
}

unsigned int SelectDimToSplit(const vector<vector<unsigned int>>& bounds, const vector<Rule>& rules) {
	auto dimCost = [&rules, &bounds](int dim) -> double {
		auto segments = GetRanges(bounds, rules, dim);

		vector<int> costs;
		for (Range s : segments) {
			int c = count_if(rules.begin(), rules.end(), [=](const Rule& r) -> bool { return r.range[dim][LowDim] <= s.first && r.range[dim][HighDim] >= s.second; });
			costs.push_back(c);
		}

		int sum = 0;
		for (int c : costs) {
			sum += c;
		}
		return ((double)sum) / costs.size();
	};
	vector<double> costs;
	for (int dim = 0; dim < rules[0].dim; dim++) {
		costs.push_back(dimCost(dim));
	}
	double minCost = *min_element(costs.begin(), costs.end());
	for (int dim = 0; dim < rules[0].dim; dim++) {
		if (costs[dim] == minCost) return dim;
	}
	return -1;
}

unsigned int SelectSplitPoint(const vector<vector<unsigned int>>& bounds, const vector<Rule>& rules, int dim) {
	vector<Range> segments = GetRanges(bounds, rules, dim);
	vector<int> costs;

	for (Range s : segments) {
		int c = count_if(rules.begin(), rules.end(), [=](const Rule& r) -> bool { return r.range[dim][LowDim] <= s.first && r.range[dim][HighDim] >= s.second; });
		costs.push_back(c);
	}
	int sum = 0;
	for (int c : costs) {
		sum += c;
	}
	int partSum = 0;
	size_t i;
	for (i = 0; i < costs.size(); i++) {
		partSum += costs[i];
		if (partSum >= sum / 2) {
			break;
		}
	}

	if (segments.size() == 1) {
		return segments[i].second;
	} else if (i + 1 == segments.size()) {
		return segments[i - 1].second;
	} else {
		return segments[i].second;
	}
}

Node* SplitRules(const vector<vector<unsigned int>>& bounds, const vector<Rule>& rules, unsigned int leafSize) {
	//PrintBounds(bounds);
	//CheckBounds(bounds);

	if (rules.size() <= leafSize) {
		return new ListNode(bounds, rules);
	} else {
		unsigned int dim = SelectDimToSplit(bounds, rules);
		unsigned int split = SelectSplitPoint(bounds, rules, dim);

		if (dim >= bounds.size() || dim < 0) {
			cout << "Bad dim!" << dim << endl;
			throw "Bad dim!";
		}


		if (split != bounds[dim][HighDim]) {
			vector<vector<unsigned int>> lbounds(bounds), rbounds(bounds);
			lbounds[dim][HighDim] = split;
			rbounds[dim][HighDim] = split + 1;
			auto lrl = LeftList(rules, dim, split);
			auto rrl = RightList(rules, dim, split);

			if (lrl.size() == rules.size() || rrl.size() == rules.size()) {
				// Won't divide anymore
				return new ListNode(bounds, rules);
			} else {
				auto lc = SplitRules(lbounds, lrl, leafSize);
				auto rc = SplitRules(rbounds, rrl, leafSize);
				return new HyperSplitNode(bounds, split, dim, lc, rc);
			}
		} else {
			return new ListNode(bounds, rules);
		}
	}
}

// **********
// HyperSplit
// **********

void HyperSplit::ConstructClassifier(const std::vector<Rule>& rules) {
	this->rules = rules;
	root = SplitRules(bounds, this->rules, leafSize);
}

int HyperSplit::ClassifyAPacket(const Packet& p) {
	return root->ClassifyAPacket(p);
}

void HyperSplit::DeleteRule(size_t index) {
	//CheckBounds(bounds);

	swap(rules[index], rules[rules.size() - 1]);
	Node* r = root->DeleteRule(rules[rules.size() - 1]);
	if (r != root) {
		delete root;
		root = r;
	}
	rules.pop_back();
}

void HyperSplit::InsertRule(const Rule& r) {
	//CheckBounds(bounds);

	rules.push_back(r);
	Node* n = root->InsertRule(leafSize, r);
	if (n != root) {
		delete root;
		root = n;
	}
}

Memory HyperSplit::MemSizeBytes() const {
	int size = 0;
	for (const vector<unsigned int> & field : bounds) {
		if (field[HighDim] == 0xFFFFFFFFu) {
			size += 5;
		} else if (field[HighDim] == 0xFFu) {
			size += 1;
		} else { //if (field[HighDim] == 0xFFFFu) {
			size += 4;
		}
		// else {
		// throw std::exception("Unknown bounds");
		// }
	}
	return root->Size(size);
}


// **********
// Split Node
// **********

int HyperSplitNode::ClassifyAPacket(const Packet& p) {
	unsigned int pt = p[splitDim];
	if (pt <= splitPoint) {
		return leftChild->ClassifyAPacket(p);
	} else {
		return rightChild->ClassifyAPacket(p);
	}
}

Node* HyperSplitNode::DeleteRule(const Rule& r) {
	//CheckBounds(bounds);

	unsigned int low = r.range[splitDim][LowDim];
	unsigned int high = r.range[splitDim][HighDim];

	if (low <= splitPoint) {
		Node* lc = leftChild->DeleteRule(r);
		if (lc != leftChild) {
			delete leftChild;
			leftChild = lc;
		}
	}
	if (high > splitPoint) {
		Node* rc = rightChild->DeleteRule(r);
		if (rc != rightChild) {
			delete rightChild;
			rightChild = rc;
		}
	}

	if (leftChild->IsEmpty()) {
		Node* res = rightChild;
		rightChild = nullptr;
		res->SetBounds(bounds);
		return res;
	} else if (rightChild->IsEmpty()) {
		Node* res = leftChild;
		leftChild = nullptr;
		res->SetBounds(bounds);
		return res;
	} else {
		return this;
	}
}

Node* HyperSplitNode::InsertRule(unsigned int leafSize, const Rule& one_rule) {
	//CheckBounds(bounds);
	unsigned int low = one_rule.range[splitDim][LowDim];
	unsigned int high = one_rule.range[splitDim][HighDim];

	if (low <= splitPoint) {
		Node* lc = leftChild->InsertRule(leafSize, one_rule);
		if (lc != leftChild) {
			delete leftChild;
			leftChild = lc;
		}
	}
	if (high > splitPoint) {
		Node* rc = rightChild->InsertRule(leafSize, one_rule);
		if (rc != rightChild) {
			delete rightChild;
			rightChild = rc;
		}
	}

	return this;
}

int HyperSplitNode::Size(int ruleSize) const {
	return NodeSize + leftChild->Size(ruleSize) + rightChild->Size(ruleSize);
}

void HyperSplitNode::SetBounds(const std::vector<std::vector<unsigned int>>& bounds) {
	CheckBounds(bounds);
	this->bounds = bounds;
}

// ********
// ListNode
// ********

int ListNode::ClassifyAPacket(const Packet& p) {
	int bestPriority = -1;
	for (size_t i = 0; i < rules.size(); i++) {
		if (rules[i].priority > bestPriority) {
			bool matches = true;
			for (int d = 0; d < rules[i].dim; d++) {
				if (!(rules[i].range[d][LowDim] <= p[d] && rules[i].range[d][HighDim] >= p[d])) {
					matches = false;
					break;
				}
			}
			if (matches) {
				bestPriority = rules[i].priority;
			}
		}
	}
	return bestPriority;
}

Node* ListNode::DeleteRule(const Rule& r) {
	//CheckBounds(bounds);
	rules.erase(remove_if(rules.begin(), rules.end(), [=](const Rule& r) -> bool {return r.priority == r.priority; }), rules.end());
	return this;
}
Node* ListNode::InsertRule(unsigned int leafSize, const Rule& r) {
	// CheckBounds(bounds);
	rules.push_back(r);
	if (rules.size() <= leafSize) {
		return this;
	} else {
		Node* n = SplitRules(bounds, rules, leafSize);
		return n;
	}
}

int ListNode::Size(int ruleSize) const {
	return NodeSize + ruleSize * rules.size();
}

void ListNode::SetBounds(const std::vector<std::vector<unsigned int>>& bounds) {
	CheckBounds(bounds);
	this->bounds = bounds;
}