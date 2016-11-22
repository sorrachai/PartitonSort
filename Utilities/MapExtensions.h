#pragma once

#include <string>
#include <unordered_map>
#include <vector>

std::unordered_map<std::string, std::string> ParseArgs(int argc, char* argv[]);

template<class K, class V>
const V& GetOrElse(const std::unordered_map<K, V> &m, const K& key, const V& def) {
	if (m.find(key) == m.end()) return def;
	else return m.at(key);
}

template<class K, class V>
V* GetOrNull(const std::unordered_map<K, V*> &m, const K& key) {
	if (m.find(key) == m.end()) return nullptr;
	else return m.at(key);
}

const std::string& GetOrElse(const std::unordered_map<std::string, std::string> &m, const std::string& key, const std::string& def);
bool GetBoolOrElse(const std::unordered_map<std::string, std::string> &m, const std::string& key, bool def);
int GetIntOrElse(const std::unordered_map<std::string, std::string> &m, const std::string& key, int def);
unsigned int GetUIntOrElse(const std::unordered_map<std::string, std::string> &m, const std::string& key, unsigned int def);
double GetDoubleOrElse(const std::unordered_map<std::string, std::string> &m, const std::string& key, double def);

void Split(const std::string &s, char delim, std::vector<std::string>& tokens);
