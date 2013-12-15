// speicherverwaltung.cpp : Defines the entry point for the console application.

#include <stdlib.h>
#include <malloc.h>
#include <iostream>
#include <list>
#include <algorithm>

using std::cout;
using std::cin;
using std::endl;

#define FIRST_FIT 0
#define BEST_FIT 1

struct memorySegment {
	unsigned int memAdress;
	unsigned int size; // size in bytes
	bool isAllocated; // Hole or Process
	int programline;
};

bool memInitialized = false;
unsigned int memBlockBeginning;
std::list<memorySegment> memSegments;
int memStrategy; // 0 = first-fit, 1 = best-fit

void myinit(unsigned int totalmem, int strategy){
	memBlockBeginning = (unsigned int)malloc(totalmem);
	memInitialized = true;
	memStrategy = strategy;

	memorySegment newMemorySegment = {memBlockBeginning, totalmem, false, 0};
	/*newMemorySegment.memAdress = memBlockBeginning;
	newMemorySegment.size = totalmem;
	newMemorySegment.isAllocated = false;
	newMemorySegment.programline = 0; */

	memSegments.push_front(newMemorySegment);
}

void mycleanup(void){

}

void *mymalloc(unsigned int size /*requested size in bytes*/, int line) {

	for(std::list<memorySegment>::iterator it = memSegments.begin(); it != memSegments.end(); ++it){
		if (it->isAllocated == false) {
			if (size <= it->size) { // first fit
				cout<<"found a fitting hole." << endl;

				memorySegment newFreeMemSeg = {it->memAdress + (size*8), it->size-size, false, 0};
				it->size = size;
				it->isAllocated = true;
				it->programline = line;
				unsigned int memAdressToReturn = it->memAdress; // the allocated segment
				memSegments.insert(++it, newFreeMemSeg); // cannot use it after this. inserts before the element at the position // TODO: make the new memSeg allocated.
				return (void*) memAdressToReturn;
			}
		}

	}
	return 0;
}

void myfree(void *addressToDelete) {
	for(std::list<memorySegment>::iterator it = memSegments.begin(); it != memSegments.end(); ++it){
		if (it->memAdress == (unsigned int)addressToDelete) {
			if (it == memSegments.begin()) { // has only one neighbor
				it->isAllocated = false;
				if (std::next(it, 1)->isAllocated == false) {
					//merge two free blocks together
					it->size += std::next(it, 1)->size;					
					memSegments.erase(std::next(it, 1));
					return;
				}				
			}
			else
			{
				it->isAllocated = false;
				if (std::prev(it, 1)->isAllocated == false) { // case f1 a2 f3
					--it; // the previous shall become the head of the freeMemSeg
					it->size += std::next(it, 1)->size;
					++it;
					if (std::next(it, 1)->isAllocated == false) {
						std::prev(it, 1)->size += std::next(it, 1)->size;
						memSegments.erase(std::next(it, 1) );
					}
					memSegments.erase(it);
					return;
				}
				else if (std::next(it, 1)->isAllocated == false) { // case a1 a2 f3 and we want to free a2
					it->size += std::next(it, 1)->size;
					memSegments.erase(std::next(it, 1) );
					return;
				}
			}

		}
	}
}

void mystatus(void){

}

int main(int argc, char* argv[])
{
	myinit(2048, FIRST_FIT);

	void *p1 = mymalloc(512, __LINE__);
	void *p2 = mymalloc(1024, __LINE__);

	cout << "did it work?" << endl;

	return 0;
}

