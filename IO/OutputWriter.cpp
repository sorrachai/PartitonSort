#include "OutputWriter.h"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iterator>

using namespace std;

string Join(const string& separator, const vector<string>& vec) {
	stringstream ss;
	for (const string& s : vec) {
		ss << s << ",";
	}
	string s = ss.str();
	s.erase(s.length() - 1);
	return s;
}

int OutputWriter::Callback(void *NotUsed, int argc, char **argv, char **azColName){
	int i;
	for (i = 0; i<argc; i++){
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	printf("\n");
	return 0;
}

int OutputWriter::CallBackCount(void* data, int count, char** rows, char**)
{
	if (count == 1 && rows) {
		*static_cast<int*>(data) = atoi(rows[0]);
		return 0;
	}
	return 1;
}

bool OutputWriter::WriteCsvFile(const string& filename, const vector<string>& header, const vector<map<string, string>>& data) {
	ofstream out(filename);
	if (out.good()) {
	//	printf("Writing to file %s\n", filename.c_str());
	} else {
		printf("Failed to open %s\n", filename.c_str());
		return false;
	}

	out << Join(",", header) << endl;

	for (auto& m : data) {
		vector<string> line;
		for (auto& f : header) {
			line.push_back(m.at(f));
		}
		out << Join(",", line) << endl;
	}
	out.close();

	if (out.good()) {
		//printf("Done writing\n");
	} else {
		printf("Problem writing\n");
	}
	
	return out.good();
}
