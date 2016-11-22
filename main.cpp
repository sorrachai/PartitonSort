#include "ElementaryClasses.h"
#include "IO/InputReader.h"
#include "IO/OutputWriter.h"
#include "Simulation.h"

#include "BruteForce.h"
#include "Trees/HyperSplit.h"
#include "Trees/HyperCuts.h"

#include "OVS/TupleSpaceSearch.h"
#include "ClassBenchTraceGenerator/trace_tools.h"
#include "SaxPac/SaxPac.h"

#include "PartitionSort/PartitionSort.h"
#include <stdio.h>


#include <assert.h>
#include <memory>
#include <chrono>
#include <string>
#include <sstream>

using namespace std;


vector<int> RunSimulatorClassificationTrial(Simulator& s, const string& name, PacketClassifier& classifier, vector<map<string, string>>& data);

pair< vector<string>, vector<map<string, string>>>  RunSimulatorOnlyClassification(const unordered_map<string, string>& args, const vector<Packet>& packets, const vector<Rule>& rules, ClassifierTests tests, const string& outfile = "");

void RunSimulatorUpdateTrial(const Simulator& s, const string& name, PacketClassifier& classifier, const vector<Request>& req,vector<map<string, string>>& data, int reps);

pair< vector<string>, vector<map<string, string>>>  RunSimulatorUpdates(const unordered_map<string, string>& args, const vector<Packet>& packets, const vector<Rule>& rules, ClassifierTests tests, const string& outfile, int repetitions = 1);

bool Validation(const unordered_map<string, PacketClassifier*> classifiers, const vector<Rule>& rules, const vector<Packet>& packets, int threshold = 10);

void RunValidation(const unordered_map<string, string>& args, const vector<Packet>& packets, const vector<Rule>& rules, ClassifierTests tests);

ClassifierTests ParseClassifier(const string& line); 
TestMode ParseMode(const string& mode);

/*
 * TODO: Parameters
 * -f Filter File
 * -p Packet File
 * -o Output file
 * -c Classifier
 * -m Mode
 * -d Database file
 */
int main(int argc, char* argv[]) {
	unordered_map<string, string> args = ParseArgs(argc, argv);

	string filterFile = GetOrElse(args, "f", "Buckets\\32k_1\\acl2_seed_1.rules");
	string packetFile = GetOrElse(args, "p", "Auto");
	string outputFile = GetOrElse(args, "o", "");

	string database = GetOrElse(args, "d", "");
	bool doShuffle = GetBoolOrElse(args, "Shuffle", true);

	//set by default
	ClassifierTests classifier = ParseClassifier(GetOrElse(args, "c", "PriorityTuple,PartitionSort"));
	TestMode mode = ParseMode(GetOrElse(args, "m", "Classification"));

	int repeat = GetIntOrElse(args, "r", 1);

	if (GetBoolOrElse(args, "?", false)) {
		printf("Arguments:\n");
		printf("\t-f <file> Filter File\n");
		printf("\t-p <file> Packet File\n");
		printf("\t-o <file> Output File\n");
		printf("\t-c <classifier> Classifier\n");
		printf("\t-m <mode> Classification Mode\n");
		printf("\t-d [<database> Database File]\n");
		printf("\t-b [<partitioning mode> Partitioning Mode]\n");
		exit(0);
	}
	
	//assign mode and classifer
	vector<Rule> rules = InputReader::ReadFilterFile(filterFile);

	vector<Packet> packets;
	//generate 1,000,000 packets from ruleset
	if (packetFile == "Auto") packets = GeneratePacketsFromRuleset(rules, 1000000);
	else if(packetFile != "") packets = InputReader::ReadPackets(packetFile);


	if (doShuffle) {
		rules = Random::shuffle_vector(rules);
	}

	switch (mode)
	{
			case ModeClassification:
			  RunSimulatorOnlyClassification(args, packets, rules, classifier, outputFile);
				break;
			case ModeUpdate:
				RunSimulatorUpdates(args, packets, rules, classifier, outputFile);
				break;
			case ModeValidation:
				RunValidation(args, packets, rules, classifier);
			        break;
	}
 
	printf("Done\n");
	return 0;
}



vector<int> RunSimulatorClassificationTrial(Simulator& s, const string& name, PacketClassifier& classifier, vector<map<string, string>>& data) {
	map<string, string> d = { { "Classifier", name } };
	printf("%s\n", name.c_str());
	auto r = s.PerformOnlyPacketClassification(classifier, d);
	data.push_back(d);
	return r;
}

pair< vector<string>, vector<map<string, string>>>  RunSimulatorOnlyClassification(const unordered_map<string, string>& args, const vector<Packet>& packets, const vector<Rule>& rules, ClassifierTests tests, const string& outfile = "") {
	printf("Classification Simulation\n");
	Simulator s(rules, packets);

	vector<string> header = { "Classifier", "ConstructionTime(ms)", "ClassificationTime(s)", "Size(bytes)", "MemoryAccess", "Tables", "TableSizes", "TableQueries", "AvgQueries" };
	vector<map<string, string>> data;

	if (tests & ClassifierTests::TestPartitionSort) {
		PartitionSort ps;
		RunSimulatorClassificationTrial(s, "PartitionSort", ps, data);
	}
	if (tests & ClassifierTests::TestPriorityTuple) {
		PriorityTupleSpaceSearch ptss;
		RunSimulatorClassificationTrial(s, "PriorityTuple", ptss, data);
	}
	if (tests & ClassifierTests::TestSaxPac) {
		sp::SaxPac sp(args);
		RunSimulatorClassificationTrial(s, "SaxPac", sp, data);
	}
	if (tests & ClassifierTests::TestHyperCuts) {
		HyperCuts hc;
		RunSimulatorClassificationTrial(s, "HyperCuts", hc, data);
	}

	if (tests & ClassifierTests::TestHyperSplit) { 
		HyperSplit hs(args);
	       	RunSimulatorClassificationTrial(s, "HyperSplit", hs, data);
	}
	if (outfile != "") {
		OutputWriter::WriteCsvFile(outfile, header, data);
	}
	return make_pair(header, data);
}

void RunSimulatorUpdateTrial(const Simulator& s, const string& name, PacketClassifier& classifier, const vector<Request>& req,vector<map<string, string>>& data, int reps) {


	map<string, string> d = { { "Classifier", name } };
	map<string, double> trial;

	printf("%s\n", name.c_str());

	for (int r = 0; r < reps; r++) { 
		s.PerformPacketClassification(classifier, req, trial);
	}
	for (auto pair : trial) {
		d[pair.first] = to_string(pair.second / reps);
	}
	data.push_back(d);
}

pair< vector<string>, vector<map<string, string>>>  RunSimulatorUpdates(const unordered_map<string, string>& args, const vector<Packet>& packets, const vector<Rule>& rules, ClassifierTests tests, const string& outfile, int repetitions = 1) {
	printf("Update Simulation\n");

	vector<string> header = { "Classifier", "UpdateTime(s)" };
	vector<map<string, string>> data;

	Simulator s(rules, packets);
	const auto req = s.SetupComputation(0, 500000, 500000);

	if (tests & ClassifierTests::TestPartitionSort) {
		PartitionSort ps;
		RunSimulatorUpdateTrial(s, "PartitionSort", ps, req, data, repetitions);
	}
	if (tests & ClassifierTests::TestPriorityTuple) {
		PriorityTupleSpaceSearch tss;
		RunSimulatorUpdateTrial(s, "PriorityTuple", tss, req, data, repetitions);
	}
	if (outfile != "") {
		OutputWriter::WriteCsvFile(outfile, header, data);
	}
	return make_pair(header, data);
}

bool Validation(const unordered_map<string, PacketClassifier*> classifiers, const vector<Rule>& rules, const vector<Packet>& packets, int threshold = 10) {
	int numWrong = 0;
	vector<Rule> sorted = rules;
	sort(sorted.begin(), sorted.end(), [](const Rule& rx, const Rule& ry) { return rx.priority >= ry.priority; });
	for (const Packet& p : packets) {
		unordered_map<string, int> results;
		int result = -1;
		for (const auto& pair : classifiers) {
			result = pair.second->ClassifyAPacket(p);
			results[pair.first] = result;
		}
		if (!all_of(results.begin(), results.end(), [=](const auto& pair) { return pair.second == result; })) {
			numWrong++;
			for (auto x : p) {
				printf("%u ", x);
			}
			printf("\n");
			for (const auto& pair : results) {
				printf("\t%s: %d\n", pair.first.c_str(), pair.second);
			}
			for (const Rule& r : sorted) {
				if (r.MatchesPacket(p)) {
					printf("\tTruth: %d\n", r.priority);
					break;
				}
			}
		}
		if (numWrong >= threshold) {
			break;
		}
	}
	return numWrong == 0;
}

void RunValidation(const unordered_map<string, string>& args, const vector<Packet>& packets, const vector<Rule>& rules, ClassifierTests tests) {
	printf("Validation Simulation\n");
	unordered_map<string, PacketClassifier*> classifiers;

	if (tests & ClassifierTests::TestPartitionSort) {
		classifiers["PartitionSort"] = new PartitionSort();
	}
	if (tests & ClassifierTests::TestPriorityTuple) {
		classifiers["PriorityTuple"] = new PriorityTupleSpaceSearch();
	}
	if (tests & ClassifierTests::TestHyperSplit) {
		classifiers["HyperSplit"] = new HyperSplit(args);
	}
	if (tests & ClassifierTests::TestHyperCuts) {
		classifiers["HyperCuts"] = new HyperCuts();
	}

	printf("Building\n");
	for (auto& pair : classifiers) {
		printf("\t%s\n", pair.first.c_str());
		pair.second->ConstructClassifier(rules);
	}

	printf("Testing\n");
	int threshold = GetIntOrElse(args, "Validate.Threshold", 10);
	if (Validation(classifiers, rules, packets, threshold)) {
		printf("All classifiers are in accord\n");
	}

	for (auto& pair : classifiers) {
		delete pair.second;
	}
}


ClassifierTests ParseClassifier(const string& line) {
	vector<string> tokens;
	Split(line, ',', tokens);
	ClassifierTests tests = ClassifierTests::TestNone;

	for (const string& classifier : tokens) {
	
	        if (classifier == "PartitionSort") {
			tests = tests | TestPartitionSort;
		}
		else if (classifier == "PriorityTuple") {
			tests = tests | TestPriorityTuple;
		}
		else if (classifier == "HyperSplit") {
			tests = tests | TestHyperSplit;
		}
		else if (classifier == "HyperCuts") {
			tests = tests | TestHyperCuts;
		}
		else if (classifier == "SaxPac") {
			tests = tests | TestSaxPac;
		}
		else if (classifier == "All") {
			tests = tests | TestAll;
		}
		else {
			printf("Unknown ClassifierTests: %s\n", classifier.c_str());
			exit(EINVAL);
		}
	}
	return tests;
}

TestMode ParseMode(const string& mode) {
	printf("%s\n", mode.c_str());
	if (mode == "Classification") {
		return ModeClassification;
	}
	else if (mode == "Update") {
		return ModeUpdate;
	}
	else if (mode == "Validation") {
		return ModeValidation;
	}
	else {
		printf("Unknown mode: %s\n", mode.c_str());
		exit(EINVAL);
	}
}
