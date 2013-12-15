#include <stdlib.h>
#include <malloc.h>
#include <iostream>
#include <list>

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
unsigned int totalMemory;
unsigned int totalAlloc;
std::list<memorySegment> memSegments;
int memStrategy; // 0 = first-fit, 1 = best-fit

void myinit(unsigned int totalmem, int strategy){
	memBlockBeginning = (unsigned int)malloc(totalmem);
	totalMemory = totalmem;
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

	std::list<memorySegment>::iterator currentBestFit;
	bool setCurrentBestFit = false;	
	for(std::list<memorySegment>::iterator it = memSegments.begin(); it != memSegments.end(); ++it){
		if (it->isAllocated == false) {
			if (size <= it->size) { // first fit
				cout<<"found a fitting hole." << endl;
				if (it->size == size) // nothing more to do if it perfectly fits.
					return (void*)it->memAdress;  // works for first-fit and best-fit
				if (memStrategy == FIRST_FIT) {
					memorySegment newFreeMemSeg = {it->memAdress + (size*8), it->size-size, false, 0};
					it->size = size;
					it->isAllocated = true;
					it->programline = line;
					unsigned int memAdressToReturn = it->memAdress; // the allocated segment
					memSegments.insert(++it, newFreeMemSeg); // cannot use it after this. inserts before the element at the position // TODO: use std::next
					totalAlloc += size;
					return (void*) memAdressToReturn;
				}
				else if (memStrategy == BEST_FIT) {
					if (setCurrentBestFit == false) {
						currentBestFit = it;
						setCurrentBestFit = true;
					}
					else if (it->size < currentBestFit->size)
						currentBestFit = it;
				}
			}
		}
	}
	if (setCurrentBestFit == true) {
		memorySegment newFreeMemSeg = {currentBestFit->memAdress + (size*8), currentBestFit->size-size, false, 0};
		currentBestFit->size = size;
		currentBestFit->isAllocated = true;
		currentBestFit->programline = line;
		memSegments.insert(std::next(currentBestFit, 1), newFreeMemSeg);
		totalAlloc += size;
		return (void*) currentBestFit->memAdress;
	}
	return 0;
}

void myfree(void *addressToDelete) {
	for(std::list<memorySegment>::iterator it = memSegments.begin(); it != memSegments.end(); ++it){
		if (it->memAdress == (unsigned int)addressToDelete) {
			totalAlloc = totalAlloc - it->size;
			if (it == memSegments.begin()) { // has only one neighbor
				it->isAllocated = false;
				if (std::next(it, 1)->isAllocated == false) {
					//merge two free blocks together
					it->size += std::next(it, 1)->size;					
					memSegments.erase(std::next(it, 1));					
				}
				return;
			}
			else if(it == memSegments.end()) { // has only one neighbor
				it->isAllocated = false;
				if (std::prev(it, 1)->isAllocated == false) {
					std::prev(it, 1)->size += it->size; // the previous shall become the head of the freeMemSeg
					memSegments.erase(it);					
				}
				return;
			}
			else
			{
				it->isAllocated = false;
				if (std::prev(it, 1)->isAllocated == false) { // case f1 a2 f3					
					std::prev(it, 1)->size += it->size; // the previous shall become the head of the freeMemSeg					
					if (std::next(it, 1)->isAllocated == false) {
						std::prev(it, 1)->size += std::next(it, 1)->size;
						memSegments.erase(std::next(it, 1) );
					}
					memSegments.erase(it);
				}
				else if (std::next(it, 1)->isAllocated == false) { // case a1 a2 f3 and we want to free a2
					it->size += std::next(it, 1)->size;
					memSegments.erase(std::next(it, 1) );					
				}
				return;
			}
		}
	}
}

void mystatus(void){
	cout<< "######## status of memory #######" << endl;
	cout<< "start: " <<std::hex<< memBlockBeginning<< " End: " <<std::hex<<memBlockBeginning+(totalMemory*8)<< endl;
	cout<< "totalmem: " << std::hex<< totalMemory << std::dec << " (" << totalMemory << ")" << endl;
	cout<< "Total allocated: " << std::hex<<totalAlloc<< " (" << std::dec << totalAlloc << ")" << endl;
	// iterate through allocations:
	int i = 0; // for numbering the allocations
	for(std::list<memorySegment>::iterator it = memSegments.begin(); it != memSegments.end(); ++it){
		if (it->isAllocated == true) {
			cout << "Allocation " << i << ":" << endl;
			cout << "Start: " <<std::hex<<it->memAdress << " End: " <<std::hex<< it->memAdress+(it->size*8) << endl;
			cout << "mymalloc(" <<std::dec<<it->size << "); in line " <<it->programline << endl;
			++i;
		}		
	}
	cout << endl;
}

int main(int argc, char* argv[])
{
	myinit(4096, BEST_FIT);

	void *p1 = mymalloc(512, __LINE__);
	void *p2 = mymalloc(1024, __LINE__);
	void *p3 = mymalloc(1024, __LINE__);

	mystatus();

	cout << "freeing memory...." << endl << endl;
	myfree(p1);
	myfree(p3);

	mystatus();

	cout << "did it work?" << endl;

	return 0;
}

