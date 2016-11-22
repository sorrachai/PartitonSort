#pragma once
#include <map>
#include <string>
#include <vector>
#include "../ElementaryClasses.h"

class OutputWriter {
public:



	static bool WriteCsvFile(const std::string& filename, const std::vector<std::string>& header, const std::vector<std::map<std::string, std::string>>& data);

private:
	static int Callback(void *NotUsed, int argc, char **argv, char **azColName);
	static int CallBackCount(void* data, int count, char** rows, char**);
};

