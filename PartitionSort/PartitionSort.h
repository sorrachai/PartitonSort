#ifndef  PSORT_H
#define  PSORT_H

#include "OptimizedMITree.h"
#include "../Simulation.h"
#include "SortableRulesetPartitioner.h"
class PartitionSort : public PacketClassifier {

public:
	~PartitionSort() {
		for (auto x : mitrees) {
			free(x);
		}
	}
	void ConstructClassifier(const std::vector<Rule>& rules)  {
		
		this->rules.reserve(rules.size());
		for (const auto& r : rules) {
			InsertRule(r);
		}
	}
	int ClassifyAPacket(const Packet& packet) {
		int result = -1;
		int query = 0;
		for (const auto& t : mitrees) {
		
			if (result > t->MaxPriority()){
				break;
			}
			query++;
			result = std::max(t->ClassifyAPacket(packet), result);
		}
		QueryUpdate(query);
		return result;
		
	}  
	void DeleteRule(size_t index);
	void InsertRule(const Rule& one_rule);
	
	Memory MemSizeBytes() const {
		int size_total_bytes = 0;
		for (const auto& t : mitrees) {
			size_total_bytes += t->MemoryConsumption();
		}
		int size_array_pointers = mitrees.size();
		int size_of_pointer = 4;
		return size_total_bytes + size_array_pointers*size_of_pointer;
	}
	int MemoryAccess() const {
		return 0;
	}
	size_t NumTables() const {
		return mitrees.size();
	}
	size_t RulesInTable(size_t index) const { return mitrees[index]->NumRules(); }

protected:
	std::vector<OptimizedMITree *> mitrees;
	std::vector<std::pair<Rule,OptimizedMITree *>> rules;

	 
	void InsertionSortMITrees() {
		int i, j, numLength = mitrees.size();
		OptimizedMITree * key;
		for (j = 1; j < numLength; j++)
		{
			key = mitrees[j];
			for (i = j - 1; (i >= 0) && (mitrees[i]-> MaxPriority() < key-> MaxPriority()); i--)
			{
				mitrees[i + 1] = mitrees[i];
			}
			mitrees[i + 1] = key;
		}
	}

};
#endif
