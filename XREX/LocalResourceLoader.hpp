#pragma once

#include "Declare.hpp"

#include <string>

class LocalResourceLoader
	: Noncopyable
{

public:
	static LocalResourceLoader& GetInstance();

public:
	
	bool LoadString(std::string const& path, std::string* result);
	bool LoadWString(std::string const& path, std::wstring* result);

private:
	LocalResourceLoader();
	~LocalResourceLoader();

};

