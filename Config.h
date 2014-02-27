#ifndef CONFIG_H
#define CONFIG_H

#include <map>
using std::map;

#include <string>
using std::string;

#include "Util.h"

class Config
{
private:
	map<string, vector<string> > config;
	string configfilename;
public:
	void Save();
	void Load(string filename);

	template<typename T>
	void SetValue(string key, uint index, T val)
	{
		if (config[key].size() <= index)
			return;
		config[key][index] = ToString(val);
		Save();
	}

	template<typename T>
	T GetValue(string key, uint index = 0)
	{
		if (config[key].size() <= index)
			return T();
		return FromString<T>(config[key][index]);
	}

	uint GetValueCount(string key)
	{
		return config[key].size();
	}
	string GetConfigFileName();
	void SetConfigFileName( string filename );
};
extern Config config;
#endif
