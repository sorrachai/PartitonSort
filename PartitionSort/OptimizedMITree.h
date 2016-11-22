#ifndef  OPTMITREE_H
#define  OPTMITREE_H

#include "red_black_tree.h"
#include "SortableRulesetPartitioner.h"
#include "../Simulation.h"


class OptimizedMITree : public ClassifierTable  {
public:
 

	OptimizedMITree(const SortableRuleset& rules) {
		numRules = 0;
		root = RBTreeCreate();
		fieldOrder = rules.GetFieldOrdering();
		maxPriority = -1;
		for (const auto& r : rules.GetRule()) {
			bool priorityChange;
			Insertion(r, priorityChange);
		}
		
	}
	OptimizedMITree(const std::vector<int>& fieldOrder) : fieldOrder(fieldOrder){
		root = RBTreeCreate();
		numRules = 0; 
		maxPriority = -1;
	}
	OptimizedMITree(const Rule& r) {
		root = RBTreeCreate();
		numRules = 0; 
		fieldOrder = SortableRulesetPartitioner::GetFieldOrderByRule(r);
		maxPriority = -1;
	}
	OptimizedMITree() {
		numRules = 0;
		root = RBTreeCreate();
		fieldOrder = { 0, 1, 2, 3 };
		maxPriority = -1;
	}
	~OptimizedMITree() {
 		  RBTreeDestroy(root);
	}
	void Insertion(const Rule& rule) {
		priorityContainer.insert(rule.priority); 
		maxPriority = std::max(maxPriority, rule.priority);
		RBTreeInsertWithPathCompression(root, rule.range, 0, fieldOrder, rule.priority);
		numRules++;
		counter++;
	}
	void Insertion(const Rule& rule, bool& priorityChange) {
	//	if (CanInsertRule(rule)) {
			priorityContainer.insert(rule.priority);
			priorityChange = rule.priority > maxPriority;
			maxPriority = std::max(maxPriority, rule.priority);
			RBTreeInsertWithPathCompression(root, rule.range, 0, fieldOrder, rule.priority);
			numRules++; 
			counter++;
   //		} 
	}
	bool TryInsertion(const Rule& rule, bool& priorityChange) {
		if (CanInsertRule(rule)) {
			counter++;
			priorityContainer.insert(rule.priority);
			priorityChange = rule.priority > maxPriority;
			maxPriority = std::max(maxPriority, rule.priority);
			RBTreeInsertWithPathCompression(root, rule.range, 0, fieldOrder, rule.priority);
			numRules++;
			return true;
		} else { return false; }
	}

	void Deletion(const Rule& rule, bool& priorityChange) {
	
		auto pit = priorityContainer.equal_range(rule.priority);
	 
		priorityContainer.erase(pit.first);
 
		if (numRules == 1) {
			maxPriority = -1;
			priorityChange = true;
		} else if (rule.priority == maxPriority) {
			priorityChange = true;
			maxPriority = *priorityContainer.rbegin();
		}
		numRules--;
		bool JustDeletedTree;
		RBTreeDeleteWithPathCompression(root, rule.range, 0, fieldOrder, rule.priority, JustDeletedTree);
	}
	bool CanInsertRule(const Rule& r) const {
		return RBTreeCanInsert(root, r.range, 0, fieldOrder);
	}
 

	//void DeleteRule(MITreeRule * mrule);
	void  Print() const { RBTreePrint(root); };
	void PrintFieldOrder() const {
		for (size_t i = 0; i < fieldOrder.size(); i++) {
			printf("%d ", fieldOrder[i]);
		}
		printf("\n");
	}
	int ClassifyAPacket(const Packet& one_packet)const {
		return  RBExactQueryIterative(root, one_packet,  fieldOrder);
		//	return   RBExactQuery(root, one_packet, 0,fieldOrder);
	}
	int ClassifyAPacket(const Packet& one_packet,int priority_so_far)const {
		return   RBExactQueryPriority(root, one_packet, 0, fieldOrder, priority_so_far);
	}
	//std::vector<MITreeRule *> MRules;

	size_t NumRules() const { return numRules; }
	int MaxPriority() const { return maxPriority; }
	bool Empty() const { return priorityContainer.empty(); }

	void ReconstructIfNumRulesLessThanOrEqualTo(int threshold = 10) {
		if (isMature) return;
		if (numRules >= threshold) {
			isMature = true;  return;
		}
		//global_counter++;
		std::vector<Rule> serialized_rules = SerializeIntoRules(); 
		auto result = SortableRulesetPartitioner::FastGreedyFieldSelectionForAdaptive(serialized_rules);
		if (!result.first) return;
		if (IsIdenticalVector(fieldOrder, result.second)) return;

		Reset();

		fieldOrder = result.second;

		for (const auto & r : serialized_rules) {
			Insertion(r);
		}
	}
	std::vector<Rule> SerializeIntoRules() const {
		return RBSerializeIntoRules(root, fieldOrder);
	}
	std::vector<Rule> GetRules() const {
		return SerializeIntoRules();
	}

	int MemoryConsumption() const{
		return CalculateMemoryConsumption(root,fieldOrder);
	}

	Memory MemSizeBytes(Memory ruleSize) const {
		return MemoryConsumption();
	}
	
private:
	bool isMature = false;
	rb_red_blk_tree * root;
	int counter = 0;
	int numRules =0;
	std::vector<int> fieldOrder;
	std::multiset<int> priorityContainer;
	int maxPriority = -1;
	bool IsIdenticalVector(const std::vector<int>& lhs, const std::vector<int>& rhs) {
		for (size_t i = 0; i < lhs.size(); i++) {
			if (lhs[i] != rhs[i]) return false;
		}
		return true;
	};


	void Reset() {

		RBTreeDestroy(root);
		numRules = 0; 
		fieldOrder.clear();
		priorityContainer.clear();
		maxPriority = -1;
		root = RBTreeCreate();

	}
	


};




#endif
