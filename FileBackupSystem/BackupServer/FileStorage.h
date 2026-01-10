#pragma once
#include <string>
class FileStorage {
public:
	void saveFile(unsigned int userId, const std::string& filename, const std::string& content);
};	
