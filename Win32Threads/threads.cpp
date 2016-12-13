#include <iostream>
#include <sstream>
#include <vector>
#include <Windows.h>

using namespace std;


//Critical section object
//We could possibly use a
//try/catch/finally here, 
//but it is not as reliable
//and removing a critical section
//from within a destructor
//read on if you don't understand
//what I'm referring to
class CustomCS
{
	CRITICAL_SECTION _cs;
public:
	CustomCS()
	{
		//this initializes the critical section
		InitializeCriticalSection(&_cs);
	}

	~CustomCS()
	{
		//this deletes the critical section
		DeleteCriticalSection(&_cs);
	}

private:
	void Start()
	{
		EnterCriticalSection(&_cs);
	}

	void Finish()
	{
		LeaveCriticalSection(&_cs);
	}
	//We also want to ensure that it is guaranteed
	//that the lock is removed after Start is called
	//So, Finish has to be called when the scope 
	//changes - we can do this by creating another class and
	//adding it as a frined here, so it has access to
	//this classes methods
	friend class CustomCSLock;
};

//Since this class is a friend of CustomCS
//we can acces the private methods
//We will call these methods when this lock
//goes out of scope.  That means we can 
//continue to use the same CustomCS while
//putting locks in differnet places, and not 
//having to worry about whether or not 
//we left a critical section
//the CustomCSLock class will automatically
//have its destructor called when the scope
//it is currently in changes, even if there is 
//an error in the code
//The same can be said about the CustomCS
//class - the DeleteCriticalSection function
//is guaranteed to be called
class CustomCSLock
{
	CustomCS& _ccs;
public:
	CustomCSLock(CustomCS& ccs) : _ccs(ccs)
	{
		_ccs.Start();
	}

	~CustomCSLock()
	{
		_ccs.Finish();
	}
};

//First, lets create the custom critical
//section we will use
CustomCS ccs;

struct ThreadData
{
	DWORD threadId;
	HANDLE threadHandle;
	int x;
	int y;
	char op;
	DWORD exitCode;

	ThreadData(int x, int y, char op) : threadId(0), threadHandle(NULL), x(x), y(y), op(op), exitCode(0) {};
};

DWORD WINAPI TheThread(LPVOID p)
{
	if (p == NULL)
	{
		return -1;
	}

	auto td = *reinterpret_cast<ThreadData*>(p);
	//let's add scope to this cout
	{
		//now let's add a lock
		CustomCSLock ccsl(ccs);
		cout << "Entered thread function -- x =" << td.x << ", y = " << td.y << "x " << td.op << " y = [to solve]" << endl;
	}//when the program gets to this point,
	 //ccsl will call its destructor, which
	 //calls Finish, which leaves this critical
	 //section

	if(td.op == '*')
	{
		return td.x * td.y;
	}

	return td.x + td.y;
}

int main()
{
	//this is the structure we are going to
	//use for the threads - we will pass one
	//element at a time in as the parameter
	//for each thread created
	vector<ThreadData> threadData;

	int y = 10;

	//loop through and throw some numbers in
	//each of the ThreadData elements
	for (int x = 0; x < 10; x++)
	{
		ThreadData tData = ThreadData(x, y, '*');
		threadData.push_back(tData);
		y++;
	}

	//loop through the thread data object and
	//start that many threads
	//I am padding the current thread object pointer
	//and a pointer to the threadId, in order to 
	//ensure the ID is passed, and that my function
	//has access to the current ThreadData object
	for(auto& thread : threadData)
	{
		//params - default security, default stack size, 
		// thread function, pointer to ThreadData object, 
		// default creation flags, pointer to ThreadObject 
		// threadId member
		thread.threadHandle = CreateThread(NULL, 0, TheThread, &thread, 0, &thread.threadId);
	}

	//create a vector to more easily handle the
	//size and array of the HANDLE objects returned
	//from CreateThread
	vector<HANDLE> threadHandles;
	for (auto& thread : threadData)
	{
		threadHandles.push_back(thread.threadHandle);
	}

	//wait for all objects
	//params - size, array, wait all, wait forever
	WaitForMultipleObjects(threadHandles.size(), threadHandles.data(), TRUE, INFINITE);

	cout << "All threads have finished" << endl << endl;

	//get all the exit codes
	for(auto& thread : threadData)
		GetExitCodeThread(thread.threadHandle , &thread.exitCode);

	//close the handles - we can't do this
	for (auto& thread : threadData)
		CloseHandle(thread.threadHandle);

	//print out results
	for (auto& thread : threadData)
	{
		cout << "x = " << thread.x << ", y = " << thread.y << " :: x " << thread.op << " y = " << thread.exitCode << endl;
	}

	cout << endl << "Press Enter to exit";
	string ss;
	getline(cin, ss);

	return 0;
}