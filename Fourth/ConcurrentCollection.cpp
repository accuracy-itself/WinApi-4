#include <list>
//#include "stdafx.h"
#include "windows.h"
//#include "iostream.h"

template<typename T>
class ConcurrentCollection
{
public:
	ConcurrentCollection()
	{
		mtx = CreateMutex(NULL, FALSE, NULL);
	}

	~ConcurrentCollection()
	{
		members.clear();
		CloseHandle(mtx);
	}

	void add(T member)
	{
		WaitForSingleObject(mtx, INFINITE);
		members.push_back(member);
		ReleaseMutex(mtx);
	}

	T pop()
	{
		WaitForSingleObject(mtx, INFINITE);
		T mmbr = members.back();
		members.pop_back();
		ReleaseMutex(mtx);
		return mmbr;
	}

	bool empty()
	{
		WaitForSingleObject(mtx, INFINITE);
		bool isempty = members.empty();
		ReleaseMutex(mtx);
		return isempty;
	}


private:
	std::list<T> members;
	HANDLE mtx;
	//std::mutex m_lock;
};