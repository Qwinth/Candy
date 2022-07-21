#pragma once
#include<string>
#include <stdlib.h>
using namespace std;

template<typename Type>
class List {
	Type* lst = new Type[1];
	size_t arrsize = 0;
	size_t capacity = 0;
public:

	void append(Type _data) {
		if (arrsize > capacity) {
			Type* newlst = new Type[arrsize + 1];
			for (int i = 0; i < arrsize; i++) {
				newlst[i] = lst[i];
			}
			capacity++;
			delete[] lst;
			newlst[arrsize] = _data;
			lst = newlst;
		}
		else {
			lst[arrsize] = _data;
		}
		arrsize++;
	}

	size_t size() {
		return arrsize;
	}

	void erase(int index) {
		Type* newlst = new Type[arrsize];
		int i, j;
		for (i = 0, j = 0; i < arrsize; i++, j++) {
			if (i == index) {
				j--;
				continue;
			}
			newlst[j] = lst[i];
		}
		arrsize--;
		capacity = arrsize;
		delete[] lst;
		lst = newlst;
	}

	void clear() {
		delete[] lst;
		lst = new Type[1];
		arrsize = 0;
		capacity = 0;
	}

	Type operator[](int index) {
		if (index >= arrsize) {
			throw string("Error: index out of range");
		}
		return this->lst[index];
	}
	
};