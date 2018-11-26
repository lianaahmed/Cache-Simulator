/***************************************************************************
 * *    Inf2C-CS Coursework 2: Cache Simulation
 * *
 * *    Instructor: Boris Grot
 * *
 * *    TA: Siavash Katebzadeh
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <time.h>
/* Do not add any more header files */

/*
 * Various structures
 */
typedef enum {FIFO, LRU, Random} replacement_p;

const char* get_replacement_policy(uint32_t p) {
    switch(p) {
    case FIFO: return "FIFO";
    case LRU: return "LRU";
    case Random: return "Random";
    default: assert(0); return "";
    };
    return "";
}

typedef struct {
    uint32_t address;
} mem_access_t;

// These are statistics for the cache and should be maintained by you.
typedef struct {
    uint32_t cache_hits;
    uint32_t cache_misses;
} result_t;


/*
 * Parameters for the cache that will be populated by the provided code skeleton.
 */

replacement_p replacement_policy = FIFO;
uint32_t associativity = 0;
uint32_t number_of_cache_blocks = 0;
uint32_t cache_block_size = 0;


/*
 * Each of the variables below must be populated by you.
 */
uint32_t g_num_cache_tag_bits = 0;
uint32_t g_cache_offset_bits= 0;
result_t g_result;


/* Reads a memory access from the trace file and returns
 * 32-bit physical memory address
 */
mem_access_t read_transaction(FILE *ptr_file) {
    char buf[1002];
    char* token = NULL;
    char* string = buf;
    mem_access_t access;

    if (fgets(buf, 1000, ptr_file)!= NULL) {
        /* Get the address */
        token = strsep(&string, " \n");
        access.address = (uint32_t)strtol(token, NULL, 16);
        return access;
    }

    /* If there are no more entries in the file return an address 0 */
    access.address = 0;
    return access;
}

void print_statistics(uint32_t num_cache_tag_bits, uint32_t cache_offset_bits, result_t* r) {
    /* Do Not Modify This Function */

    uint32_t cache_total_hits = r->cache_hits;
    uint32_t cache_total_misses = r->cache_misses;
    printf("CacheTagBits:%u\n", num_cache_tag_bits);
    printf("CacheOffsetBits:%u\n", cache_offset_bits);
    printf("Cache:hits:%u\n", r->cache_hits);
    printf("Cache:misses:%u\n", r->cache_misses);
    printf("Cache:hit-rate:%2.1f%%\n", cache_total_hits / (float)(cache_total_hits + cache_total_misses) * 100.0);
}

/*
 *
 * Add any global variables and/or functions here as needed.
 *
 */

 // Counters for Hits and Misses

    int hits = 0;
    int misses = 0;


int main(int argc, char** argv) {
    time_t t;
    /* Intializes random number generator */
    /* Important: *DO NOT* call this function anywhere else. */
    srand((unsigned) time(&t));
    /* ----------------------------------------------------- */
    /* ----------------------------------------------------- */

    /*
     *
     * Read command-line parameters and initialize configuration variables.
     *
     */
    int improper_args = 0;
    char file[10000];
    if (argc < 6) {
        improper_args = 1;
        printf("Usage: ./mem_sim [replacement_policy: FIFO LRU Random] [associativity: 1 2 4 8 ...] [number_of_cache_blocks: 16 64 256 1024] [cache_block_size: 32 64] mem_trace.txt\n");
    } else  {
        /* argv[0] is program name, parameters start with argv[1] */
        if (strcmp(argv[1], "FIFO") == 0) {
            replacement_policy = FIFO;
        } else if (strcmp(argv[1], "LRU") == 0) {
            replacement_policy = LRU;
        } else if (strcmp(argv[1], "Random") == 0) {
            replacement_policy = Random;
        } else {
            improper_args = 1;
            printf("Usage: ./mem_sim [replacement_policy: FIFO LRU Random] [associativity: 1 2 4 8 ...] [number_of_cache_blocks: 16 64 256 1024] [cache_block_size: 32 64] mem_trace.txt\n");
        }
        associativity = atoi(argv[2]);
        number_of_cache_blocks = atoi(argv[3]);
        cache_block_size = atoi(argv[4]);
        strcpy(file, argv[5]);
    }
    if (improper_args) {
        exit(-1);
    }
    assert(number_of_cache_blocks == 16 || number_of_cache_blocks == 64 || number_of_cache_blocks == 256 || number_of_cache_blocks == 1024);
    assert(cache_block_size == 32 || cache_block_size == 64);
    assert(number_of_cache_blocks >= associativity);
    assert(associativity >= 1);

    printf("input:trace_file: %s\n", file);
    printf("input:replacement_policy: %s\n", get_replacement_policy(replacement_policy));
    printf("input:associativity: %u\n", associativity);
    printf("input:number_of_cache_blocks: %u\n", number_of_cache_blocks);
    printf("input:cache_block_size: %u\n", cache_block_size);
    printf("\n");

    /* Open the file mem_trace.txt to read memory accesses */
    FILE *ptr_file;
    ptr_file = fopen(file,"r");
    if (!ptr_file) {
        printf("Unable to open the trace file: %s\n", file);
        exit(-1);
    }

    /* result structure is initialized for you. */
    memset(&g_result, 0, sizeof(result_t));

    /* Do not delete any of the lines below.
     * Use the following snippet and add your code to finish the task. */

    /* You may want to setup your Cache structure here. */
    
    // Total number of sets

    int setsNum = number_of_cache_blocks / associativity;

    // Initialise offsetBits size
    int g_cache_offset_bits = log2(cache_block_size);
    
    // Initialise index bits size
    int indexSize = log2(setsNum);

    // Initialise tagBits size
    int g_num_cache_tag_bits = 32 - (indexSize + g_cache_offset_bits);

    // 2D array for Cache

    uint32_t **cache;
    cache = malloc(setsNum*sizeof(uint32_t*));
    
    for (int i = 0; i < setsNum; i++){
        *(cache + i) = malloc(associativity*sizeof(uint32_t));
    }

    // 2D array to check valid values in the Cache array

    uint32_t **valid;
    valid = malloc(setsNum*sizeof(uint32_t*));

    for (int i = 0; i < setsNum; i++){
        *(valid + i) = malloc(associativity*sizeof(uint32_t));
    }
    // Initialise all values in the valid array to 0

    for (int i = 0; i < setsNum; i++){
        for (int j = 0; j < associativity; j++){
            valid[i][j] = 0;
        }
    }

    // 2D array used to keep track of when each cache position was accessed

    uint32_t **queue;
    queue = malloc(setsNum*sizeof(uint32_t*));

    for (int i = 0; i < setsNum; i++){
        *(queue + i) = malloc(associativity*sizeof(uint32_t));
    }

    // Initialise all values in the queue array to -1

    for (int i = 0; i < setsNum; i++){
        for (int j = 0; j < associativity; j++){
            queue[i][j] = -1;
        }
    }

    mem_access_t access;

    /* Loop until the whole trace file has been read. */
    
    while(1) {

        access = read_transaction(ptr_file);
        
        // If no transactions left, break out of loop.

        if (access.address == 0){
            break;
        }

        uint32_t currentTag = access.address >> (32 - g_num_cache_tag_bits);


        uint32_t currentIndex = 0;

        if(setsNum> 1){
            currentIndex = ((access.address << g_num_cache_tag_bits) - 1) >> (g_num_cache_tag_bits + g_cache_offset_bits);
        }
      
        // Add your code here!

        // Loops through every position in the cache

        for (int i = 0; i < associativity; i++){

            // If at the last position in the cache
            if(i == (associativity - 1)){

                // check if there isn't an item in the cache

                if(valid[currentIndex][i] == 0){

                    // if there isn't:
                    // add the current tag to the cache
                    // set valid to 1
                    // add 1 to the queue position

                    cache[currentIndex][i] = currentTag;
                    valid[currentIndex][i] = 1;

                    //if using LRU
                    if(replacement_policy == LRU){
                        queue[currentIndex][i]++;
                    }
                    //if using FIFO
                    else if(replacement_policy == FIFO){

                        // set queue to 0
                        queue[currentIndex][i] = 0;

                        //Loop through every other position i the queue row and add 1
                        for(int j = 0; j < (associativity - 1); j++){

                            queue[currentIndex][j]++;

                        }

                    }

                    break;

                    // check if there is an item in the cache

                } else if (valid[currentIndex][i] == 1) {

                    // If there is, check if the tag matches the current tag in the cache
                    if(cache[currentIndex][i] == currentTag){
                        
                        // add one to the hits counter
                        hits++;

                        //if using LRU
                        if(replacement_policy == LRU){

                            // loop through every value > -1 in the queue array and add 1
                            for(int j = 0; j < associativity; j++){
                                // Check if the cache has been accessed yet
                                if(queue[currentIndex][j] != -1){

                                    // If it has, add 1 to thee queue and break
                                    queue[currentIndex][j]++;
                                    break;

                                }

                            }
                        
                        } 
                        // If using FIFO
                        else if(replacement_policy == FIFO){

                            //Set queue as 0
                            queue[currentIndex][i] = 0;

                            // Loop through every other position in the queue row and add 1
                            for(int j = 0; j < (associativity - 1); j++){

                                queue[currentIndex][j]++;

                            }
                        }
                        
                        break;

                    } 
                    else {

                        // if there isn't, add 1 to the misses counter
                        misses++;

                        // Check which replacement policy is being used

                        //If using LRU
                        if(replacement_policy == LRU){
                            
                            //stores the least recently used address value in the queue array
                            int LRU_add = 0;

                            //stores the location of the least recently used address
                            int LRU_i;
                            int LRU_j;

                            // Looping through each row of the queue array
                            for(int j = 0; j < associativity; j++){
                                
                                //If the current value in queue 
                                if(queue[currentIndex][j] > LRU_add){

                                    LRU_add = queue[currentIndex][j];
                                    LRU_i = currentIndex;
                                    LRU_j = j;

                                } else {

                                    break;

                                }
                            }
                            
                            // Replace the tag in the least recently used position
                            // in the cache with the current tag

                            cache[LRU_i][LRU_j] = currentTag;

                            // If using FIFO

                        } else if (replacement_policy == FIFO){

                            int FIFO_add = 0;

                            int FIFO_i;
                            int FIFO_j;

                            for(int j = 0; j < associativity; j++){

                                if (queue[currentIndex][j] > FIFO_add){
                                    
                                    FIFO_add = queue[currentIndex][j];
                                    FIFO_i = currentIndex;
                                    FIFO_j = j;

                                } else {

                                    break;

                                }

                            }

                            // Replace the tag in the first in position
                            // in the cache with the current tag

                            cache[FIFO_i][FIFO_j] = currentTag;
                        }
                        // If random
                        else if(replacement_policy == Random){

                            //Generates a random number between 0 and (associativity - 1)
                            int rNum = rand() % associativity;

                            // replaces the random tag with the current tag
                            cache[currentIndex][rNum] = currentTag;

                        }

                        break;

                    }
                }

                // If not at last position in the cache
                // check if there isn't an item in the cache

            } else if(valid[currentIndex][i] == 0){

                // if there isn't:
                // add the current tag to the cache
                // set valid to 1
                // add 1 to the queue position

                cache[currentIndex][i] = currentTag;
                valid[currentIndex][i] = 1;
               
                //If using LRU add 1 to queue
                if(replacement_policy == LRU){
                    queue[currentIndex][i]++;
                }
                //if using FIFO
                else if(replacement_policy == FIFO){

                    // set queue to 0
                    queue[currentIndex][i] = 0;

                    for(int j = 0; j < associativity; j++){

                        // If not at the same queue position as the one we just set to 0
                        // add 1 to the queue position
                        if(j != i){
                            queue[currentIndex][j]++;
                        }

                    }

                }
                    
                break;

                // check if there is an item in the cache
            } else if (valid[currentIndex][i] == 1) {

                // if there is, add 1 to the hits counter and the queue position and break
                if(cache[currentIndex][i] == currentTag){
                    
                    // add one to the hits counter
                    hits++;

                     //if using LRU
                        if(replacement_policy == LRU){

                            // loop through every value > -1 in the queue array and add 1
                            for(int j = 0; j < associativity; j++){
                                // Check if the cache has been accessed yet
                                if(queue[currentIndex][j] != -1){

                                    // If it has, add 1 to thee queue and break
                                    queue[currentIndex][j]++;
                                    break;

                                }

                            }
                        
                        } 
                        // If using FIFO
                        else if(replacement_policy == FIFO){

                            //Set queue as 0
                            queue[currentIndex][i] = 0;

                            // Loop through every other position in the queue row and add 1
                            for(int j = 0; j < associativity; j++){
                                
                                 // If not at the same queue position as the one we just set to 0
                                 // add 1 to the queue position
                                if(i != j){
                                    queue[currentIndex][j]++;
                                }
                
                            }
                        }
                    
                    break;
                }

            }
        }

    }

    
    // Sets cache_hits to the value in hits
    // Sets cache_missses to the value in misses
    g_result.cache_hits = hits;
    g_result.cache_misses = misses;
    
    // clears the memory in the cache, valid and queue arrays
    free(cache);
    free(valid);
    free(queue);
    
    /* Do not modify code below. */
    /* Make sure that all the parameters are appropriately populated. */
    print_statistics(g_num_cache_tag_bits, g_cache_offset_bits, &g_result);

    /* Close the trace file. */
    fclose(ptr_file);
    return 0;
}
