#ifndef APP_H
#define APP_H

#include "Curl.h"
#include "AppOpenCL.h"
#include "CPUMiner.h"

class App
{
private:
	struct ServerData
	{
		uint current_id;
		uint last_tried;
	};

	uint current_server_id;
	string nickbase;

	Curl curl;
	OpenCL opencl;
	CPUMiner cpuminer;

	clock_t workupdate;

	uint getworks;

	void SetupCurrency();

	void Parse_SLC(string data);
	void Parse_BTC(string data);
public:
	void Main(vector<string> args);
	void Parse(string data);
	void LoadServers();
};

#endif
