#ifndef  BF_H
#define  BF_H

#include "Simulation.h"

class BruteForce : public PacketClassifier {
public:
	BruteForce(){}
	void ConstructClassifier(const std::vector<Rule>& r) {
		rules = r;
		rules.reserve(100000);
	}


 
	int MemoryAccess() const {
		return 0;
	}
	size_t NumTables() const {
		return 0;
	}
	size_t RulesInTable(size_t tableIndex) const{
		return 0;
	}


	int ClassifyAPacket(const Packet& one_packet) {
		
		int result = -1;
		auto IsPacketMatchToRule = [](const Packet& p, const Rule& r) {
			for (int i = 0; i < r.dim; i++) {
				if (p[i] < r.range[i][0]) return 0;
				if (p[i] > r.range[i][1]) return 0;
			}
			return 1;
		};
		
		for (size_t j = 0; j < rules.size(); j++) {
			if (IsPacketMatchToRule(one_packet, rules[j])) {
				result = std::max(rules[j].priority, result);
			}
		}
		return result;
	}

	void DeleteRule(size_t i) {
		if (i < 0 || i >= rules.size()) {
			printf("Warning index delete rule out of bound: do nothing here\n");
			printf("%d vs. size: %d", i, rules.size());
			return;
		}
		if (i != rules.size() -1)
		rules[i]=std::move(rules[rules.size() - 1]);
		rules.pop_back();
	}
	void InsertRule(const Rule& one_rule) { 
		rules.push_back(one_rule);
	}
	int Size() const {
		return 0;
	}
	std::vector<Rule> GetRules() const {
		return rules;
	}
private:
	std::vector<Rule> rules;

};

#endif
