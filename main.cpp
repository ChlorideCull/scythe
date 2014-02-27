#include "Global.h"
#include "App.h"

int main(int argc, char* argv[])
{
	try
	{
		vector<string> args;
		for(int i=0; i<argc; ++i)
		{
			args.push_back(argv[i]);
		}
		App app;
		app.Main(args);
	}
	catch(string s)
	{
		cout << "Error: " << s << endl;
	}
	catch(std::exception s)
	{
		cout << "Error: " << s.what() << endl;
	}
	catch(...)
	{
		cout << "Error." << endl;
	}
}
