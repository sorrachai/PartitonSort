#pragma once

#include "../Simulation.h"
#include "TreeUtils.h"

class HyperCutsNode {
public:
	int depth = 0;
	bool isProblematic = false;

	std::list<Rule*> classifier;
	std::list<HyperCutsNode*> children;
	std::vector<HyperCutsNode*> childArray;

	std::vector<Range> bounds;
	std::vector<int> cuts;
};

class HyperCutsHelper {
public:
	HyperCutsNode* CreateTree(const std::list<Rule*>& classifier);

private:
	std::list<HyperCutsNode*> CalcCuts(HyperCutsNode* node);
	std::list<HyperCutsNode*> CalcNumCuts1D(HyperCutsNode* root, size_t dim);
	std::list<HyperCutsNode*> CalcNumCuts2D(HyperCutsNode* root, size_t* dims);
	void CalcDimensionsToCut(HyperCutsNode* node, std::vector<bool>& selectDims);

	size_t leafSize = 8;
	double spfac = 4;

	bool isHyperCuts = true;

	bool compressionOn = false;
	bool binningOn = false;
	bool mergingOn = false;
};

class HyperCuts : public PacketClassifier {
public:
	virtual void ConstructClassifier(const std::vector<Rule>& rules);
	virtual int ClassifyAPacket(const Packet& packet);
	virtual void DeleteRule(size_t index) { 
		printf("Can't delete rules.\n");
	}
	virtual void InsertRule(const Rule& rule) { 
		printf("Can't insert rules.\n");
	}
	virtual Memory MemSizeBytes() const { return 0; } // TODO
	virtual int MemoryAccess() const { return 0; } // TODO
	virtual size_t NumTables() const { return 1; }
	virtual size_t RulesInTable(size_t tableIndex) const { return rules.size(); }

private:
	std::vector<Rule> rules;

	HyperCutsNode* root;
};
