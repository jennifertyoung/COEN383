#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define NUM_JOBS 50

typedef struct _process {
    int processName; // Name is a character A-Z
    int processNameNumber; // An integer is appended to the job name, incrementing every 26 jobs
    int totalPages; // Process size in pages, must be 5, 11, 17, or 31 pages
    int arrivalTime; // Random int between 0 and 59 (seconds)
    int serviceDuration; // Random integer between 1 and 5
    int usedPages[31]; // Each index is a page belonging to the process, and the value is the page used from the page list
    int lastPageReferenced; // The process's page number that it last made a reference to
} process;

typedef struct _node {
    process job; // Process information
    struct node * nextPtr; // Pointer to next process node in the list
} node;

typedef struct _pageNode {
    int pageNumber; // Numbers pages 0-99
    int busy; // 1 if page is being used, 0 if free
    process job;
    struct pageNode * nextPtr;
} pageNode;

process processArray[NUM_JOBS];

int randomNumberOfPages() {
    int r = rand() % 4;
    if (r == 0)
        return 5;
    else if (r == 1)
        return 11;
    else if (r == 2)
        return 17;
    else
        return 31;
}

int compareArrivalTime(const void * a, const void * b) {
    process *processA = (process *) a;
    process *processB = (process *) b;
    return (processA->arrivalTime - processB->arrivalTime);
}

// Generates processes, stores them in an array, and sorts according to arrival time
void generateSortedProcesses() {
    int i = 0;
    int j = 0;
    int k = 0;

    for (i = 0; i < NUM_JOBS; ++i) {
        processArray[i].processName = ('A' + (i % 26));
        if ((i != 0) && (i % 26 == 0))
            ++j;
        processArray[i].processNameNumber = j;
        processArray[i].totalPages = randomNumberOfPages();
        processArray[i].arrivalTime = rand() % 60;
        processArray[i].serviceDuration = (rand() % 5) + 1;
        int x;
        for (x = 0; x < 31; x++)
            processArray[i].usedPages[x] = NULL;
        processArray[i].lastPageReferenced = 0;
    }
    qsort(processArray, NUM_JOBS, sizeof(process), compareArrivalTime);
}

// Uses the elements from processArray to create the ordered linked list and returns pointer of the root node
node * copyArrayToList() {
    node *rootPtr;
    node *nodePtr;
    int i;

    rootPtr = malloc(sizeof(node));
    rootPtr->nextPtr = 0;
    rootPtr->job = processArray[0];
    nodePtr = rootPtr;

    for (i = 1; i < NUM_JOBS; ++i) {
        nodePtr->nextPtr = malloc(sizeof(node));
        nodePtr = nodePtr->nextPtr;
        nodePtr->nextPtr = 0;
        nodePtr->job = processArray[i];
    }

    return rootPtr;
}

void printJobQueue(node * nodePtr) {
    while (nodePtr) {
        printf("Process %c%d details: %d, %d, %d\n", nodePtr->job.processName, nodePtr->job.processNameNumber, nodePtr->job.totalPages, nodePtr->job.arrivalTime, nodePtr->job.serviceDuration);
        nodePtr = nodePtr->nextPtr;
    }
}

// Creates and initializes the free page list, returns pointer of the root page node
pageNode * createFreePageList() {
    pageNode *rootPtr;
    pageNode *nodePtr;
    int i;

    rootPtr = malloc(sizeof(pageNode));
    rootPtr->nextPtr = 0;
    rootPtr->pageNumber = 0;
    rootPtr->busy = 0;
    nodePtr = rootPtr;

    for (i = 1; i < 100; ++i) {
        nodePtr->nextPtr = malloc(sizeof(pageNode));
        nodePtr = nodePtr->nextPtr;
        nodePtr->nextPtr = 0;
        nodePtr->pageNumber = i;
        nodePtr->busy = 0;
    }
    return rootPtr;
}

// Returns the number of free pages in the page list
int amountOfFreePages(pageNode * nodePtr) {
    int freePages = 0;
    while(nodePtr) {
        if (nodePtr->busy == 0)
            ++freePages;
        nodePtr = nodePtr->nextPtr;
    }
    return freePages;
}

// Gives the first free page on the page list to the process needing it and returns its page number
int allocateFreePage(node * nodePtr, pageNode * pageNodePtr) {
    while ((pageNodePtr->nextPtr) && (pageNodePtr->busy == 1))
        pageNodePtr = pageNodePtr->nextPtr;

    if (pageNodePtr->pageNumber < 100) {
        pageNodePtr->job = nodePtr->job;
        pageNodePtr->busy = 1;
        return pageNodePtr->pageNumber;
    }

    return NULL;
}

void printPageList(pageNode * nodePtr) {
    while (nodePtr) {
        printf("Page %d information: ", nodePtr->pageNumber);
        if (nodePtr->busy == 1)
            printf("Busy -> %c%d\n", nodePtr->job.processName, nodePtr->job.processNameNumber);
        else
            printf("Free\n");
        nodePtr = nodePtr->nextPtr;
    }
}

// Updates the number of the page the process next references
void nextReference(node * nodePtr) {
    int localityReferenceChance = rand() % 100;

    if (localityReferenceChance <= 69) {
        int localityDifference = rand() % 3;
        if (localityDifference == 0)
            nodePtr->job.lastPageReferenced = (nodePtr->job.lastPageReferenced + 1) % nodePtr->job.totalPages;
        else if (localityDifference == 1) {
            nodePtr->job.lastPageReferenced = (nodePtr->job.lastPageReferenced - 1) % nodePtr->job.totalPages;
            if (nodePtr->job.lastPageReferenced < 0)
                nodePtr->job.lastPageReferenced += nodePtr->job.totalPages;
        }
    }

    else
        nodePtr->job.lastPageReferenced = (nodePtr->job.lastPageReferenced + rand()) % nodePtr->job.totalPages;
}

int main() {
    srand(time(NULL));
    generateSortedProcesses();
    node * listPtr = copyArrayToList();
    printJobQueue(listPtr);
    pageNode * pagePtr = createFreePageList();

    int time;
    for (time = 0; time < 60; ++time) {
        while (listPtr && (listPtr->job.arrivalTime == time)) {
            if (amountOfFreePages(pagePtr) >= 4) {
                listPtr->job.usedPages[0] = allocateFreePage(listPtr, pagePtr);
                int i = 0;
                for (i = 0; i < 3; ++i) {
                    nextReference(listPtr);
                    if (listPtr->job.usedPages[listPtr->job.lastPageReferenced] == NULL)
                        listPtr->job.usedPages[listPtr->job.lastPageReferenced] = allocateFreePage(listPtr, pagePtr);
                }
            }
            listPtr = listPtr->nextPtr;
        }
    }

    printPageList(pagePtr);
    printf("Final number of free pages: %d\n", amountOfFreePages(pagePtr));

    return 0;
}
