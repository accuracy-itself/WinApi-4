#include "ConcurrentCollection.cpp"

#include <Windows.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <list>
namespace fs = std::filesystem;

ConcurrentCollection<std::string> result;
std::string word = "uiu";
long tasksnumber = 0;
CRITICAL_SECTION CounterLock;
CONDITION_VARIABLE CounterNull;

std::string FindString(std::string path, std::string word)
{
	std::string line;
	std::ifstream stream;
	stream.open(path);
	int count = 0;
	while (std::getline(stream, line))
	{
		int start = 0;
		std::string p = line;
		while ((start = line.find(word, start)) != std::string::npos)
		{
			start++;
			count++;
		}
	}

	std::string answer = path + " : " + std::to_string(count);
	stream.close();
	return answer;
}

void NTAPI FindStringWork(PTP_CALLBACK_INSTANCE instance, PVOID context, PTP_WORK work)
{
	std::string path = *(std::string*)context;
	result.add(FindString(path, word));
	//std::cout << "here\n";
	InterlockedDecrement(&tasksnumber);

	if (InterlockedCompareExchange(&tasksnumber, 0, 0) == 0)
		WakeAllConditionVariable(&CounterNull);
	return;
}

int main()
{
	InitializeCriticalSection(&CounterLock);
	InitializeConditionVariable(&CounterNull);
	InterlockedExchange(&tasksnumber, 0);

	std::cout << "Type your string!\n";
	std::cin >> word;
	std::string dirpath = "D:\\1";
	std::filesystem::path directoryPath(dirpath);
	std::filesystem::directory_iterator it{ directoryPath };
	while (it != std::filesystem::directory_iterator{})
	{
		if (!it->is_regular_file())
			continue;

		std::string* filepath = new std::string(it->path().string());
		TP_WORK* work = CreateThreadpoolWork(FindStringWork, (PVOID)filepath, NULL);

		if (work == NULL)
			printf("Error %d in CreateThreadpoolWork", GetLastError());
		else
		{
			InterlockedIncrement(&tasksnumber);
			SubmitThreadpoolWork(work);
			//WaitForThreadpoolWorkCallbacks(work, FALSE);
			//CloseThreadpoolWork(work);
		}

		*it++;
	}
	
	EnterCriticalSection(&CounterLock);
	while (InterlockedCompareExchange(&tasksnumber, 0, 0) != 0)
		SleepConditionVariableCS(&CounterNull, &CounterLock, INFINITE);
	LeaveCriticalSection(&CounterLock);

	while (!result.empty())
	{
		std::cout << result.pop() << std::endl;
	}
}
