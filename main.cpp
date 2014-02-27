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
		cout << "Fatal error: " << s << endl;
	}
	catch(std::exception s)
	{
		cout << "Fatal Error: " << s.what() << endl;
	}
	catch(...)
	{
		cout << "wtfpafeokopfrthkgrjopij" << endl;
	}
}
