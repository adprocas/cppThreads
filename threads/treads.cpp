#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>

using namespace std;

int number = 0;
std::mutex g_mtxSection;

void threadFunction(void) {
}

void incrementNumber(void) {
	++number;
	cout << "Number: " << number << endl;
}

void incrementNumberMutex(void) {
	{
		std::lock_guard<std::mutex> lk(g_mtxSection);
		++number;
		cout << "Number: " << number << endl;
	}
}

int main() {
	//This scope block uses regular threads without any type of thread locking or safety
	{
		cout << "Threads without making threadsafe" << endl;

		vector<thread> threads;

		for (int i = 0; i < 10; ++i) {
			//thread t(incrementNumber);
			threads.push_back(thread(incrementNumber));
		}

		for(vector<thread>::iterator it = threads.begin(); it != threads.end(); ++it) {
			it->join();
		}
	}

	//This scope block uses a method that utilizes mutex for thread safety
	{
		cout << "Threads -- threadsafe using mutex" << endl;

		vector<thread> threads;

		for (int i = 0; i < 10; ++i) {
			//thread t(incrementNumber);
			threads.push_back(thread(incrementNumberMutex));
		}

		for (vector<thread>::iterator it = threads.begin(); it != threads.end(); ++it) {
			it->join();
		}
	}
}