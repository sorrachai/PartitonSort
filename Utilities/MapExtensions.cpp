#include "MapExtensions.h"

#include <sstream>

using namespace std;

unordered_map<string, string> ParseArgs(int argc, char* argv[]) {
	unordered_map<string, string> results;
	for (int i = 1; i < argc; i++) {
		string arg = argv[i];
		vector<string> tokens;
		Split(arg, '=', tokens);
		if (tokens.size() == 2) {
			results[tokens[0]] = tokens[1];
		} else if (tokens.size() == 1) {
			results[tokens[0]] = "1";
		} else {
			printf("Wrong number of tokens! %s\n", arg.c_str());
		}
	}
	return results;
}

const string& GetOrElse(const unordered_map<string, string> &m, const string& key, const string& def) {
	if (m.find(key) == m.end()) return def;
	else return m.at(key);
}

bool GetBoolOrElse(const unordered_map<string, string> &m, const string& key, bool def) {
	if (m.find(key) == m.end()) return def;
	else {
		string s = m.at(key);
		if (s == "true") return 1;
		else if (s == "false") return 0;
		return std::stoi(m.at(key)) != 0;
	}
}

int GetIntOrElse(const unordered_map<string, string> &m, const string& key, int def) {
	if (m.find(key) == m.end()) return def;
	else return std::stoi(m.at(key));
}

unsigned int GetUIntOrElse(const unordered_map<string, string> &m, const string& key, unsigned int def) {
	if (m.find(key) == m.end()) return def;
	else return std::stoul(m.at(key));
}

double GetDoubleOrElse(const unordered_map<string, string> &m, const string& key, double def) {
	if (m.find(key) == m.end()) return def;
	else return std::stod(m.at(key));
}

void Split(const string &s, char delim, vector<string>& tokens) {
	stringstream ss(s);
	string item;
	while (getline(ss, item, delim)) {
		tokens.push_back(item);
	}
}