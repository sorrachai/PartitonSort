#include "HyperCuts.h"

using namespace std;
using namespace TreeUtils;

void PrintRange(const Range& r) {
	printf("[%u - %u]", r.low, r.high);
}

void RemoveRedund(HyperCutsNode* node) {
	RemoveRedund(node->classifier, node->bounds);
}

void RegionCompaction(HyperCutsNode* node) {
	for (size_t d = 0; d < node->bounds.size(); d++) {
		list<Point> f;
		for (const Rule* r : node->classifier) {
			if (r->range[d][LowDim] < node->bounds[d].low) {
				f.push_back(node->bounds[d].low);
			} else {
				f.push_back(r->range[d][LowDim]);
			}
			if (r->range[d][HighDim] > node->bounds[d].high) {
				f.push_back(node->bounds[d].high);
			} else {
				f.push_back(r->range[d][HighDim]);
			}
		}
		f.sort();
		node->bounds[d].low = f.front();
		node->bounds[d].high = f.back();
	}
}

Point ComputeSpan(Range bounds, int cuts) {
	return (bounds.high - bounds.low) / cuts + 1;
}

void CopyNode(HyperCutsNode* src, HyperCutsNode* dest) {
	dest->depth = src->depth;
	dest->bounds = src->bounds;
	dest->classifier = src->classifier;
	dest->children = src->children;
	dest->cuts = src->cuts;
}

void SetBoundary(HyperCutsNode* src, HyperCutsNode* dest, const vector<int>& offsets) {
	for (size_t d = 0; d < src->bounds.size(); d++) {
		// Normal case
		Point interval = ComputeSpan(src->bounds[d], src->cuts[d]);
		dest->bounds.push_back(Range());

		dest->bounds[d].low = src->bounds[d].low + offsets[d] * interval;

		if (offsets[d] == src->cuts[d] - 1) {
			dest->bounds[d].high = src->bounds[d].high;
		} else {
			if (interval == 0) {
				dest->bounds[d].high = dest->bounds[d].low;
			} else {
				dest->bounds[d].high = dest->bounds[d].low + interval - 1;
			}
		}
		dest->cuts.push_back(-1);
	}
}

bool IsPresent(const vector<Range>& bounds, const Rule* rule) {
	for (size_t d = 0; d < bounds.size(); d++) {
		if (rule->range[d][HighDim] < bounds[d].low || rule->range[d][LowDim] > bounds[d].high) {
			return false;
		}
	}
	return true;
}

list<HyperCutsNode*> NodeMerging(HyperCutsNode* node) {
	list<HyperCutsNode*> newlist = node->children;

	// Addition to convert each node in the lookup array to the merged node
	unordered_map<HyperCutsNode*, HyperCutsNode*> listing;

	for (auto i1 = newlist.begin(); i1 != newlist.end(); i1++) {
		auto i2 = i1;
		listing[*i1] = *i1;
		i2++;
		while (i2 != newlist.end()) {
			if (AreSameRules((*i1)->classifier, (*i2)->classifier)) {
				for (size_t d = 0; d < node->bounds.size(); d++) {
					(*i1)->bounds[d].low = min((*i1)->bounds[d].low, (*i2)->bounds[d].low);
					(*i1)->bounds[d].high = max((*i1)->bounds[d].high, (*i2)->bounds[d].high);
				}
				listing[*i2] = *i1;
				delete(*i2);
				i2 = newlist.erase(i2);
			} else {
				i2++;
			}
		}
	}

	for (size_t i = 0; i < node->childArray.size(); i++) {
		node->childArray[i] = listing[node->childArray[i]];
	}

	return newlist;
}

void MoveRulesUp(HyperCutsNode* node) {
	list<Rule*> rulesMovedUp = node->children.front()->classifier; // Start with this set
	bool emptyIntersect = true;

	for (HyperCutsNode* child : node->children) {
		if (emptyIntersect) {
			list<Rule*> setToCheck = rulesMovedUp;
			rulesMovedUp.clear();
			for (Rule* rs : setToCheck) {
				for (Rule* rc : child->classifier) {
					if (rs == rc) {
						rulesMovedUp.push_back(rs);
						break;
					}
				}
			}
			if (rulesMovedUp.empty()) {
				emptyIntersect = false;
			}
		}
	}

	// Remove pushed-up rules from children
	if (emptyIntersect) {
		for (Rule* r : rulesMovedUp) {
			for (HyperCutsNode* child : node->children) {
				child->classifier.erase(find(child->classifier.begin(), child->classifier.end(), r));
			}
			// Update statistics
		}
	}

	//for (const Rule* r : rulesMovedUp) {
	//	r->Print();
	//}

	node->classifier = rulesMovedUp;
}

void HyperCutsHelper::CalcDimensionsToCut(HyperCutsNode* node, vector<bool>& selectDims) {
	vector<int> uniqueElements;

	for (size_t d = 0; d < node->bounds.size(); d++) {
		list<Range> rangeList;
		for (Rule* rule : node->classifier) {
			bool found = false;
			Range check;
			check.low = max(rule->range[d][LowDim], node->bounds[d].low);
			check.high = min(rule->range[d][HighDim], node->bounds[d].high);
			for (Range range : rangeList) {
				if (check.low == range.low && check.high == range.high) {
					found = true;
					break;
				}
			}
			if (!found) {
				rangeList.push_back(check);
			}
		}
		uniqueElements.push_back(rangeList.size());
	}

	double average = 0;
	int dimsCount = 0;
	for (size_t d = 0; d < node->bounds.size(); d++) {
		if (node->bounds[d].high > node->bounds[d].low) {
			average += uniqueElements[d];
			dimsCount++;
		}
	}
	average /= dimsCount;

	int maxUnique = *max_element(uniqueElements.begin(), uniqueElements.end());
	for (size_t d = 0; d < node->bounds.size(); d++) {
		selectDims.push_back(false);
	}

	int dimCount = 0;
	for (size_t d = 0; d < node->bounds.size(); d++) {
		if (node->bounds[d].high > node->bounds[d].low) {
			if (isHyperCuts) {
				if (uniqueElements[d] >= average) {
					selectDims[d] = true;
					dimCount++;
					if (dimCount == 2) break;
				}
			} else {
				if (uniqueElements[d] == maxUnique) {
					selectDims[d] = true;
					break;
				}
			}
		}
	}
	
}

list<HyperCutsNode*> HyperCutsHelper::CalcNumCuts1D(HyperCutsNode* root, size_t dim) {
	int nump = 0;
	int spmf = int(floor(root->classifier.size() * spfac));
	int sm = 0;

	int prevDepth = -1;

	HyperCutsNode* top = new HyperCutsNode();
	CopyNode(root, top);

	list<HyperCutsNode*> childList;
	childList.push_back(top);

	while (!childList.empty()) {
		HyperCutsNode* node = childList.front();

		if (prevDepth != node->depth) {
			if (sm < spmf) {
				nump++;
				sm = 1 << nump;
				prevDepth = node->depth;
			} else {
				break;
			}
		}

		for (int k = 0; k < 2; k++) {
			node->cuts[dim] = 2;
			HyperCutsNode* child = new HyperCutsNode();
			child->depth = node->depth + 1;

			vector<int> offsets;
			for (size_t d = 0; d < node->bounds.size(); d++) {
				if (d == dim) {
					offsets.push_back(k);
				} else {
					offsets.push_back(0);
				}
			}

			SetBoundary(node, child, offsets);

			for (size_t d = 0; d < node->bounds.size(); d++) {
				child->cuts[d] = 1;
			}

			for (Rule* r : node->classifier) {
				if (IsPresent(child->bounds, r)) {
					child->classifier.push_back(r);
				}
			}

			// Rows and columns

			// Is compressed?

			sm += child->classifier.size();

			// TODO : error check : singleton space and rule

			childList.push_back(child);
		}
		childList.pop_front();
		delete node;
	}

	root->cuts[dim] = 1 << nump;

	childList.sort([=](HyperCutsNode* n1, HyperCutsNode* n2) {
		return n1->bounds[dim].low <= n2->bounds[dim].low;
	});

	return childList;
}

list<HyperCutsNode*> HyperCutsHelper::CalcNumCuts2D(HyperCutsNode* root, size_t* dims) {
	// row / col
	Point spans[2];
	int nump[2];
	for (int i = 0; i < 2; i++) {
		nump[i] = 0;
		spans[i] = root->bounds[dims[i]].high - root->bounds[dims[i]].low + 1;
	}

	int spmf = int(floor(root->classifier.size() * spfac));
	int sm = 0;

	int prevDepth = -1;
	unsigned short chosen = 1;

	list<HyperCutsNode*> childList;
	HyperCutsNode* top = new HyperCutsNode();
	CopyNode(root, top);
	childList.push_back(top);

	while (!childList.empty()) {
		HyperCutsNode* node = childList.front();
		if (prevDepth != node->depth) {
			if (sm < spmf) {
				chosen = chosen ^ 1;
				//if (pow(2, nump[chosen]) >= spans[chosen]) {
				//	// Can't split this dimension anymore
				//	chosen = chosen ^ 1;
				//}
				nump[chosen]++;
				sm = 1 << (nump[0] + nump[1]);
				prevDepth = node->depth;
			} else {
				break;
			}
		}

		for (int k = 0; k < 2; k++) {
			node->cuts[dims[chosen]] = 2;
			node->cuts[dims[chosen ^ 1]] = 1;

			HyperCutsNode* child = new HyperCutsNode();
			child->depth = node->depth + 1;

			vector<int> offsets;
			for (size_t d = 0; d < node->bounds.size(); d++) {
				if (d == dims[chosen]) {
					offsets.push_back(k);
				} else {
					offsets.push_back(0);
				}
			}

			SetBoundary(node, child, offsets);

			for (size_t d = 0; d < node->bounds.size(); d++) {
				child->cuts[d] = 1;
			}

			for (Rule* r : node->classifier) {
				if (IsPresent(child->bounds, r)) {
					child->classifier.push_back(r);
				}
			}

			// Rows and columns

			// Is compressed?

			sm += child->classifier.size();

			// TODO : error check : singleton space and rule

			childList.push_back(child);
		}

		childList.pop_front();
		delete node;
	}

	root->cuts[dims[0]] = (1 << nump[0]);
	root->cuts[dims[1]] = (1 << nump[1]);

	if (compressionOn) {
		// False
	}
	childList.sort([=](HyperCutsNode* n1, HyperCutsNode* n2) {
		if (n1->bounds[dims[0]].low != n2->bounds[dims[0]].low) {
			return n1->bounds[dims[0]].low <= n2->bounds[dims[0]].low;
		} else {
			return n1->bounds[dims[1]].low <= n2->bounds[dims[1]].low;
		}
	});
	return childList;
}

list<HyperCutsNode*> HyperCutsHelper::CalcCuts(HyperCutsNode* node) {
	vector<bool> selectDims;
	size_t chosenDims[2];
	int chosenCount = 0;

	CalcDimensionsToCut(node, selectDims);

	for (size_t d = 0; d < node->bounds.size(); d++) {
		if (selectDims[d]) {
			chosenDims[chosenCount++] = d;
		}
	}

	if (chosenCount > 2) {
		printf("Error: More than 2 dimensions are cut!\n");
		exit(1);
	}

	if (chosenCount > 1 && !isHyperCuts) {
		printf("Error: HiCut: more than 1 dimension is cut!\n");
		exit(1);
	}

	if (chosenCount == 0) {
		printf("Error: at least 1 dimension needs to be cut!\n");
		exit(1);
	}

	list<HyperCutsNode*> children;

	if (chosenCount == 2) {
		children = CalcNumCuts2D(node, chosenDims);
	} else if (chosenCount == 1) {
		children = CalcNumCuts1D(node, chosenDims[0]);
	} else {
		printf("Error: should be impossible to get here\n");
		exit(1);
	}

	return children;
}

HyperCutsNode* HyperCutsHelper::CreateTree(const list<Rule*>& classifier) {
	list<HyperCutsNode*> worklist;

	HyperCutsNode* root = new HyperCutsNode();
	root->depth = 1;
	root->bounds.push_back({ 0, 0xFFFFFFFF });
	root->bounds.push_back({ 0, 0xFFFFFFFF });
	root->bounds.push_back({ 0, 0xFFFF });
	root->bounds.push_back({ 0, 0xFFFF });
	root->bounds.push_back({ 0, 0xFF });

	for (size_t i = 0; i < root->bounds.size(); i++) {
		root->cuts.push_back(1);
	}

	root->classifier.insert(root->classifier.end(), classifier.begin(), classifier.end());

	RemoveRedund(root);

	if (root->classifier.size() > leafSize) {
		worklist.push_back(root);
	}

	int maxDepth = 0;
	while (!worklist.empty()) {
		HyperCutsNode* node = worklist.back();
		worklist.pop_back();

		if (node->depth > maxDepth) {
		  //	printf("Depth: %d\n", node->depth);
			maxDepth = node->depth;
		}

		if (isHyperCuts) {
			//RegionCompaction(node);
		}

		list<HyperCutsNode*> childList = CalcCuts(node);

		for (HyperCutsNode* n : childList) {
			n->depth = node->depth + 1;
			/*for (size_t i = 0; i < n->bounds.size(); i++) {
				n->cuts.push_back(0);
			}*/
			node->children.push_back(n);
			node->childArray.push_back(n);
		}
		childList.clear();

		if (compressionOn) {
			// False
		}

		node->classifier.clear();
		if (!compressionOn && isHyperCuts) {
			//MoveRulesUp(node);
		}

		//list<HyperCutsNode*> toPush = NodeMerging(node);
		list<HyperCutsNode*> toPush = node->children;

		for (HyperCutsNode* n : toPush) {
			RemoveRedund(n);
		}

		for (HyperCutsNode* n : toPush) {
			if (n->classifier.size() > leafSize) {
				bool areIdentical = true;
				for (size_t d = 0; d < n->bounds.size(); d++) {
					if (n->bounds[d].low != node->bounds[d].low || n->bounds[d].high != node->bounds[d].high) {
						areIdentical = false;
					}
				}
				if (areIdentical && n->classifier.size() == node->classifier.size()) {
					printf("Warning: parent and child are identical with %lu rules\n", node->classifier.size());
					node->isProblematic = true;
					//NodeStats(n);
					//ClearMem(n);
				} else {
					worklist.push_back(n);
				}
			} else {
				if (!n->classifier.empty()) {
					n->isProblematic = false;
					//NodeStats(n);
				}
				//ClearMem(n);
			}
		}
		node->isProblematic = false;
		//NodeStats(n);
		//ClearMem(n);
	}

	return root;
}

void HyperCuts::ConstructClassifier(const vector<Rule>& rules) {
	this->rules = rules;
	SortRules(this->rules);
	list<Rule*> rl;
	for (Rule & r : this->rules) {
		rl.push_back(&r);
	}
	HyperCutsHelper helper;
	root = helper.CreateTree(rl);
}

int Classify(const HyperCutsNode* node, const Packet& packet) {
	int priority = -1;

	//for (Range r : node->bounds) {
	//	PrintRange(r);
	//}
	//printf("\n");

	for (const Rule* r : node->classifier) {
		//printf("%d -> ", r->priority);
		//r->Print();
		if (r->MatchesPacket(packet)) {
			//printf("Match: %d\n", r->priority);
			priority = max(priority, r->priority);
			break;
		}
	}

	if (!node->childArray.empty()) {
		//printf("Array\n");
		int index = 0;
		for (size_t d = 0; d < node->bounds.size(); d++) {
			if (node->cuts[d] > 1) {
				//printf("node->bounds = [%u, %u]\n", node->bounds[d].low, node->bounds[d].high);
				//printf("node->cuts = %d\n", node->cuts[d]);
				Point span = ComputeSpan(node->bounds[d], node->cuts[d]);
				Point x = packet[d] - node->bounds[d].low;
				int i = x / span;
				index *= node->cuts[d];
				index += i;
			}
		}
		//for (size_t i = 0; i < node->childArray.size(); i++) {
		//	HyperCutsNode* n = node->childArray[i];
		//	bool isNode = true;
		//	for (size_t d = 0; d < n->bounds.size(); d++) {
		//		if (n->bounds[d].low > packet[d] || n->bounds[d].high < packet[d]) {
		//			isNode = false;
		//			break;
		//		}
		//	}
		//	if (isNode) {
		//		printf("Correct index: %u\n", i);
		//		for (Range & r : n->bounds) {
		//			PrintRange(r);
		//		}
		//		printf("\n");
		//		break;
		//	}
		//}

		//printf("Index: %d / %u\n", index, node->childArray.size());
		priority = max(priority, Classify(node->childArray[index], packet));
	}

	return priority;
}

int HyperCuts::ClassifyAPacket(const Packet& packet) {
	int p = Classify(root, packet);
	//printf("Result - %d\n", p);
	return p;
}
