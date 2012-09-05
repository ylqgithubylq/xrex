#include "XREX.hpp"

#include "LocalResourceLoader.hpp"

#include <fstream>
#include <vector>




using std::vector;
using std::string;
using std::wstring;
using std::ifstream;
using std::wifstream;
using std::ios;


LocalResourceLoader& LocalResourceLoader::GetInstance()
{
	static LocalResourceLoader instance_;
	return instance_;
}

LocalResourceLoader::LocalResourceLoader()
{
}


LocalResourceLoader::~LocalResourceLoader()
{
}

bool LocalResourceLoader::LoadString(string const& path, string* result)
{
	ifstream file(path, ios::in | ios::binary);

	if (file)
	{
		file.seekg(0, ios::end);
		int32 length = static_cast<int32>(file.tellg());
		file.seekg(0, ios::beg);

		string temp;
		temp.resize(length + 1);
		file.read(&temp[0], length);

		*result = move(temp);

		return true;
	}
	return false;
}

bool LocalResourceLoader::LoadWString(string const& path, wstring* result)
{
	wifstream file(path, ios::in | ios::binary);

	if (file)
	{
		file.seekg(0, ios::end);
		int32 length = static_cast<int32>(file.tellg());
		file.seekg(0, ios::beg);

		wstring temp;
		temp.resize(length + 1);
		file.read(&temp[0], length);

		*result = move(temp);

		return true;
	}
	return false;
}


