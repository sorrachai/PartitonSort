#pragma once
#include "../Simulation.h"
#include "GridOfTries.h"

#include <unordered_map>
#include <vector>

namespace sp {
	bool IsOrderIndependent(const std::vector<Rule*> &rules, const std::vector<int> &fields);

	class SaxPac : public PacketClassifier {
	public:
		SaxPac(const std::unordered_map<std::string, std::string>& args);
		~SaxPac();

		void ConstructClassifier(const std::vector<Rule>& rules);
		int ClassifyAPacket(const Packet& one_packet);
		void DeleteRule(size_t index);
		void InsertRule(const Rule& one_rule);
		Memory MemSizeBytes() const;
		int MemoryAccess() const;

		virtual size_t NumTables() const { return numTables; };
		virtual size_t RulesInTable(size_t tableIndex) const { return tries[tableIndex].CountRules(); };
	private:
		int fieldLimit;
		int numTables = 0;

		bool useSpecial;

		std::vector<Rule> rules;
		std::vector<GridOfTries> tries;
	};
}



