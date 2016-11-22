#include "PartitionSort.h"


void PartitionSort::InsertRule(const Rule& one_rule) {

 
	for (auto mitree : mitrees)
	{
		bool prioritychange = false;
		
		bool success = mitree->TryInsertion(one_rule, prioritychange);
		if (success) {
			
			if (prioritychange) {
				InsertionSortMITrees();
			}
			mitree->ReconstructIfNumRulesLessThanOrEqualTo(10);
			rules.push_back(std::make_pair(one_rule, mitree));
			return;
		}
	}
	bool priority_change = false;
	 
	auto tree_ptr = new OptimizedMITree(one_rule);
	tree_ptr->TryInsertion(one_rule, priority_change);
	rules.push_back(std::make_pair(one_rule, tree_ptr));
	mitrees.push_back(tree_ptr);  
	InsertionSortMITrees();
}


void PartitionSort::DeleteRule(size_t i){
	if (i < 0 || i >= rules.size()) {
		printf("Warning index delete rule out of bound: do nothing here\n");
		printf("%lu vs. size: %lu", i, rules.size());
		return;
	}
	bool prioritychange = false;

	OptimizedMITree * mitree = rules[i].second; 
	mitree->Deletion(rules[i].first, prioritychange); 
 
	if (prioritychange) {
		InsertionSortMITrees();
	}


	if (mitree->Empty()) {
		mitrees.pop_back();
		delete mitree;
	}


	if (i != rules.size() - 1) {
		rules[i] = std::move(rules[rules.size() - 1]);
	}
	rules.pop_back();


}
