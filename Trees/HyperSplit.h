#pragma once
#ifndef  HyperSplit_H
#define  HyperSplit_H

#include "../Simulation.h"

//#include <string>

class Node {
public:
	virtual ~Node(){}

	virtual int ClassifyAPacket(const Packet& one_packet) = 0;
	virtual Node* DeleteRule(const Rule& one_rule) = 0;
	virtual Node* InsertRule(unsigned int leafSize, const Rule& r) = 0;
	virtual int Size(int ruleSize) const = 0;
	virtual bool IsEmpty() const = 0;

	virtual void SetBounds(const std::vector<std::vector<unsigned int>>& bounds) = 0;
};

class NodeTable : public ClassifierTable {
public:
	NodeTable(Node* root, int numRules, int maxPriority, unsigned int leafSize) :
		root(root), numRules(numRules), maxPriority(maxPriority), leafSize(leafSize)
	{
	}
	virtual int ClassifyAPacket(const Packet& packet) const {
		return root->ClassifyAPacket(packet);
	}
	virtual void Insertion(const Rule& rule, bool& priorityChange) {
		if (rule.priority > maxPriority) {
			maxPriority = rule.priority;
			priorityChange = true;
		} else {
			priorityChange = false;
		}
		root->InsertRule(leafSize, rule);
	}
	virtual void Deletion(const Rule& rule, bool& priorityChange) {
		priorityChange = false;
		root->DeleteRule(rule);
	}

	virtual bool CanInsertRule(const Rule& rule) const {
		return true;
	}

	virtual size_t NumRules() const { 
		return numRules;
	}
	virtual int MaxPriority() const {
		return maxPriority;
	}

	virtual Memory MemSizeBytes(Memory ruleSize) const {
		return root->Size(ruleSize);
	}

	virtual std::vector<Rule> GetRules() const {
		return std::vector<Rule>(); // TODO
	}

private:
	Node* root;
	size_t numRules;
	int maxPriority;
	unsigned int leafSize;
};

class HyperSplit : public PacketClassifier {
public:
	HyperSplit(std::vector<std::vector<unsigned int>> bounds, int leafSize) : bounds(bounds), leafSize(leafSize){}
	HyperSplit(int leafSize = 8) : leafSize(leafSize) {
		bounds.push_back(std::vector<unsigned int>{0, 0xFFFFFFFFu});
		bounds.push_back(std::vector<unsigned int>{0, 0xFFFFFFFFu});
		bounds.push_back(std::vector<unsigned int>{0, 0xFFFFu});
		bounds.push_back(std::vector<unsigned int>{0, 0xFFFFu});
		bounds.push_back(std::vector<unsigned int>{0, 0xFFu});
	}
	HyperSplit(const std::unordered_map<std::string, std::string>& args) : HyperSplit(GetIntOrElse(args, "HyperSplit.Leaf", 8)) {}
	virtual ~HyperSplit() {
		delete root;
	}

	void ConstructClassifier(const std::vector<Rule>& rules);
	int ClassifyAPacket(const Packet& p);
	void DeleteRule(size_t index);
	void InsertRule(const Rule& r);
	Memory MemSizeBytes() const;
	int MemoryAccess() const {
		printf("warning unimplemented MemoryAccess()\n");
		return 0;
	}
	size_t NumTables() const { return 1; }
	size_t RulesInTable(size_t index) const { return rules.size(); }
private:
	std::vector<std::vector<unsigned int>> bounds;
	Node* root;
	std::vector<Rule> rules;
	int leafSize;
};

class HyperSplitNode : public Node {
public:
	//SplitNode(unsigned int split, int dim) : splitPoint(split), splitDim(dim) {}
	/**
	* Constructor: Takes ownership of the designated pointers
	*/
	HyperSplitNode(const std::vector<std::vector<unsigned int>>& bounds, unsigned int split, int dim, Node* lc, Node* rc)
		: splitPoint(split), splitDim(dim), leftChild(lc), rightChild(rc), bounds(bounds) {
		if (splitDim > 10 || splitDim < 0) {
			std::cout << splitDim << std::endl;
			throw "Bad dim in constructor!";
		}
		//CheckBounds(this->bounds, "Split-Const");
	}
	virtual ~HyperSplitNode() {
		delete leftChild;
		delete rightChild;
	}

	int ClassifyAPacket(const Packet& p);
	Node* DeleteRule(const Rule& r);
	Node* InsertRule(unsigned int leafSize, const Rule& r);
	int Size(int ruleSize) const;
	bool IsEmpty() const { return false; }

	void SetBounds(const std::vector<std::vector<unsigned int>>& bounds);
private:

	const unsigned int splitPoint;
	const int splitDim;
	Node* leftChild;
	Node* rightChild;

	std::vector<std::vector<unsigned int>> bounds;
};

class ListNode : public Node {
public:
	//ListNode(const std::vector<std::vector<unsigned int>>& bounds) : bounds(bounds) {}
	ListNode(const std::vector<std::vector<unsigned int>>& bounds, const std::vector<Rule>& rules) : rules(rules), bounds(bounds) { }

	int ClassifyAPacket(const Packet& p);
	Node* DeleteRule(const Rule& r);
	Node* InsertRule(unsigned int leafSize, const Rule& r);
	int Size(int ruleSize) const;
	bool IsEmpty() const { return rules.empty(); }
	void SetBounds(const std::vector<std::vector<unsigned int>>& bounds);
private:

	std::vector<Rule> rules;
	std::vector<std::vector<unsigned int>> bounds;
};

Node* SplitRules(const std::vector<std::vector<unsigned int>>& bounds, const std::vector<Rule>& rules, unsigned int leafSize);

#endif