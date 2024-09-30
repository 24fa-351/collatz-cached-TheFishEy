#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>


#define POLICY_NONE 0
#define POLICY_LRU 1
#define POLICY_RR 2

// cache entry 
typedef struct {
    int number;
    int steps;

    int lastAccessTime; 


} CacheEntry;

CacheEntry *cache = NULL;

int cacheSize = 0;
int cachePolicy = POLICY_NONE;
int cacheHits = 0, cacheMisses = 0, currentTime = 0;
int cacheCount = 0;
int maxSteps = 10000;  // having issues of freezing/infinite loop without this?

// collatz steps
int collatzSteps(int n) {

    int steps = 0;

    while (n != 1) {

        if (steps >= maxSteps) {

            printf("You have reached max steps for number: %d\n", n);

            // return error for max step
            return -1; 
        }
        if (n % 2 == 0) {
            n /= 2;

        } else {
            n = 3 * n + 1; 
            //n = 3n

        }
        steps++;
    }

    return steps;
}

// cache search
int findInCache(int number) {
    for (int i = 0; i < cacheCount; i++) {

        if (cache[i].number == number) {

            cache[i].lastAccessTime = currentTime++;  
            
            cacheHits++;

            return cache[i].steps;  // Cache hit
        }
    }
    return -1;  
}

// LRU or RR
void insertIntoCache(int number, int steps) { 

    cacheMisses++;
    
    //if cache has space:
    if (cacheCount < cacheSize) {

        cache[cacheCount].number = number;
        cache[cacheCount].steps = steps;

        cache[cacheCount].lastAccessTime = currentTime++;

        cacheCount++;

    } 
    
    // else: replacment with LRU or RR
    else {

        int index = 0;
        // if LRU: find least recently used -> smallest time
        if (cachePolicy == POLICY_LRU) {
            
            int minAccessTime = cache[0].lastAccessTime;

            index = 0;


            for (int i = 1; i < cacheSize; i++) {

                if (cache[i].lastAccessTime < minAccessTime) {

                    minAccessTime = cache[i].lastAccessTime;
                    index = i;
                }
            }
        } 
        
        // or RR
        else if (cachePolicy == POLICY_RR) {
            // use rand() to find random index to replace
            // 
            index = rand() % cacheSize;
        }

        cache[index].number = number;
        cache[index].steps = steps;
        cache[index].lastAccessTime = currentTime++;
    }
}

// cache wrapper
int collatzWithCache(int number) {

    int steps = findInCache(number);


    if (steps == -1) {
        // if miss: insert
        steps = collatzSteps(number);
        
        if (steps != -1) {  
            insertIntoCache(number, steps);
        }
    }
    return steps;
}


int main(int argc, char *argv[]) {

    if (argc != 6) {

        printf("usage percentage is: %s N MIN MAX cache_size cache_policy (ru/rr)\n", argv[0]);

        return 1;
    }

    int N = atoi(argv[1]);
    int MIN = atoi(argv[2]);
    int MAX = atoi(argv[3]);

    cacheSize = atoi(argv[4]);
    char *policy = argv[5];

    if (strcmp(policy, "none") == 0) {

        cachePolicy = POLICY_NONE;
    } 
    else if (strcmp(policy, "lru") == 0) {

        cachePolicy = POLICY_LRU;
    } 
    else if (strcmp(policy, "rr") == 0) {

        cachePolicy = POLICY_RR;
    } 
    else {

        printf("Try: 'none', 'lru' or 'rr'.\n");


        return 1;
    }

    // cache initialization:
    // if:c
    if (cachePolicy != POLICY_NONE) {

        cache = (CacheEntry *)malloc(cacheSize * sizeof(CacheEntry));
        
        if (!cache) {

            printf("memory failed to allocate for cache.\n");

            return 1;
        }
    }


    // CSV file (open)
    FILE *csvFile = fopen("output.csv", "a"); //w rewriets every time

    if (!csvFile) {
        printf("Unable to open file\n\n");

        return 1;
    }
    fprintf(csvFile, "Cache Policy\tCache Hit\tCache Miss\tCache Hit Rate\n");




    // generating rand() for test
    srand(time(NULL));

    // test n rand numbers between min max
    for (int i = 0; i < N; i++) {

        int randomNum = MIN + rand() % (MAX - MIN + 1);

        int steps;

        if (cachePolicy == POLICY_NONE) {
            steps = collatzSteps(randomNum);
        } 
        // else cahce
        else { 
            steps = collatzWithCache(randomNum);
        }
        if (steps != -1) {
            printf("Number: %d, Steps: %d\n", randomNum, steps);

            //fprintf(csvFile, "%d, %d\n", randomNum, steps);
        } 
        else {
            printf("Number: %d exceeded max steps\n", randomNum);
        }
    }




    // print hit%
    if (cachePolicy != POLICY_NONE) {

        double cacheHitPercent = (cacheHits * 100.0) / (cacheHits + cacheMisses);

        printf("Cache Hit: %d, Cache Miss: %d, Cache Hit Rate: %.2f%%\n", cacheHits, cacheMisses, cacheHitPercent);

        // print also to csv
        //also hindsight, i used tabs to make it more readable (in the actual csv reader), bu thtis might be bad for exports.
        fprintf(csvFile, "%s\t\t\t\t%d\t\t\t%d\t\t%.2f%%\n", (cachePolicy == POLICY_LRU) ? "LRU" : "RR", cacheHits, cacheMisses, cacheHitPercent);
    }

    // free cache
    if (cache) {

        free(cache);
    }

    fclose(csvFile);


    return 0;
}
