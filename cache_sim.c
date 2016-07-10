#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>

typedef enum {dm, fa} cache_map_t;
typedef enum {uc, sc} cache_org_t;
typedef enum {instruction, data} access_t;

typedef struct {
    uint32_t address;
    access_t accesstype;
} mem_access_t;


// DECLARE CACHES AND COUNTERS FOR THE STATS HERE

/* Block
 *
 * Holds an integer that states the validity of the bit (0 = invalid,
 * 1 = valid), the tag being held, and another integer that states if
 * the bit is dirty or not (0 = clean, 1 = dirty).
 */

typedef struct {
    int tag;
    int valid;
} Block;

  int offset_size = 6;
  int index_size = 0;
  int tag_size = 0;
  int b = 1;
  int fifo1 = 0;
  int fifo2 = 0;
	int hits1 = 0;
  int hits2 = 0;
	int accesses1 = 0;
  int accesses2 = 0;
	double hit_rate = 0.0;
	int numBlocks;
  int rcount = 0;

Block *cache1, *cache2;

uint32_t cache_size;
uint32_t block_size = 64;
cache_map_t cache_mapping;
cache_org_t cache_org;

// **********FUNCTIONS************

void directedMapped(mem_access_t access){
    int accTag = access.address << offset_size;
    int index = accTag >> tag_size;
    accTag = accTag << index_size;

    if (cache_org == uc){
          if(cache1[index].tag == accTag){
              accesses1++;
              hits1++;
          }
          else {
                 accesses1++;
                 cache1[index].tag = accTag;
          }
    }
    else {
          if (access.accesstype == instruction)
            if(cache1[index].tag == accTag){
                  accesses1++;
                  hits1++;
            }
            else {
                  accesses1++;
                  cache1[index].tag = accTag;
            }
          else {
              if(cache2[index].tag == accTag){
                    accesses2++;
                    hits2++;
              }
              else {
                    accesses2++;
                    cache2[index].tag = accTag;
              }
            }
      }
    }

    void fullyAssociative(mem_access_t access){
      int accTag = access.address << offset_size;

      if (cache_org == uc){
        int i;

        for (i = 0; i < rcount; ++i)
          if (accTag == cache1[i].tag && cache1[i].valid == 0){
                accesses1++;
                hits1++;
                cache1[i].valid = 1;
          }
          if (cache1[i].valid == 0 && rcount < numBlocks){
            cache1[rcount++].tag = accTag;
            fifo1++; accesses1++;
          }
          if (cache1[i].valid == 0 && rcount == numBlocks){
            fifo1++; accesses1++;
            cache1[fifo1%(rcount+1)].tag = accTag;
          }
      }
      else {
        int i, rcount1 = 0, rcount2 = 0;

        if (access.accesstype == instruction)
          for (i = 0; i < rcount1; ++i){
            if (accTag == cache1[i].tag && cache1[i].valid == 0){
                  accesses1++;
                  hits1++;
                  cache1[i].valid = 1;
            }
            if (cache1[i].valid == 0 && rcount1 < numBlocks/2){
              cache1[rcount1++].tag = accTag;
              fifo1++; accesses1++;
            }
            if (cache1[i].valid == 0 && rcount1 >= numBlocks/2){
              fifo1++; accesses1++;
              cache1[fifo2%(rcount1+1)].tag = accTag;
            }
      }
      else for (i = 0; i < rcount2; ++i){
        if (accTag == cache2[i].tag && cache2[i].valid == 0){
              accesses2++;
              hits2++;
              cache1[i].valid = 1;
        }
        if (cache2[i].valid == 0 && rcount2 < numBlocks/2){
          cache2[rcount2++].tag = accTag;
          fifo2++; accesses2++;
        }
        if (cache2[i].valid == 0 && rcount2 == numBlocks/2){
          fifo2++; accesses2++;
          cache2[fifo2%(rcount2+1)].tag = accTag;
        }
      }
    }
}

/* Reads a memory access from the trace file and returns
 * 1) access type (instruction or data access
 * 2) memory address
 */
mem_access_t read_transaction(FILE *ptr_file) {
    char buf[1000];
    char* token;
    char* string = buf;
    mem_access_t access;

    if (fgets(buf,1000, ptr_file)!=NULL) {

        /* Get the access type */
        token = strsep(&string, " \n");
        if (strcmp(token,"I") == 0) {
            access.accesstype = instruction;
        } else if (strcmp(token,"D") == 0) {
            access.accesstype = data;
        } else {
            printf("Unkown access type\n");
            exit(0);
        }

        /* Get the access type */
        token = strsep(&string, " \n");
        access.address = (uint32_t)strtol(token, NULL, 16);

        return access;
    }

    /* If there are no more entries in the file,
     * return an address 0 that will terminate the infinite loop in main
     */
    access.address = 0;
    return access;
}


void main(int argc, char** argv){

    /* Read command-line parameters and initialize:
     * cache_size, cache_mapping and cache_org variables
     */
    if ( argc != 4 ) { /* argc should be 2 for correct execution */
        printf("Usage: ./cache_sim [cache size: 128-4096] [cache mapping: dm|fa] [cache organization: uc|sc]\n");
        exit(0);
    }
    else  {
        /* argv[0] is program name, parameters start with argv[1] */

        /* Set cache size */
        cache_size = atoi(argv[1]);

        /* Set Cache Mapping */
        if (strcmp(argv[2], "dm") == 0) {
            cache_mapping = dm;
        } else if (strcmp(argv[2], "fa") == 0) {
            cache_mapping = fa;
        } else {
            printf("Unknown cache mapping\n");
            exit(0);
        }

        /* Set Cache Organization */
        if (strcmp(argv[3], "uc") == 0) {
            cache_org = uc;
        } else if (strcmp(argv[3], "sc") == 0) {
            cache_org = sc;
        } else {
            printf("Unknown cache organization\n");
            exit(0);
        }
    }

    /* Open the file mem_trace.txt to read memory accesses */
    FILE *ptr_file;
    ptr_file =fopen("mem_trace.txt","r");
    if (!ptr_file) {
        printf("Unable to open the trace file\n");
        exit(1);
    }

    /* Calculate number of Blocks */
    numBlocks = (int)(cache_size / block_size);

    /* Calculate number of bits for index*/
    if (cache_mapping == dm)
      while (b <= numBlocks){
          b *= 2;
          index_size++;
      }
    /* Calculate number of bits for tag*/
    tag_size = numBlocks - offset_size - index_size;

    cache1 = (Block*) calloc(numBlocks, sizeof (Block) );
    cache2 = (Block*) calloc(numBlocks, sizeof (Block) );

    /* Loop until whole trace file has been read */
    mem_access_t access;
    while(1) {
        access = read_transaction(ptr_file);
        if(cache_mapping == fa)
            fullyAssociative(access);
        else directedMapped(access);
        //If no transactions left, break out of loop
        if (access.address == 0)
            break;
          }

    /* Print the statistics */
    if (cache_org == uc) printf("U.accesses: %d\nU.hits: %d\nU.hit rate: %1.3f\n", accesses1, hits1, (double) hits1/accesses1);
    else {
      printf("I.accesses: %d\nI.hits: %d\nI.hit rate: %1.3f\n", accesses1, hits1, (double) hits1/accesses1);
      printf("\n");
      printf("D.accesses: %d\nD.hits: %d\nD.hit rate: %1.3f\n", accesses2, hits2, (double) hits2/accesses2);
    }

    /* Close the trace file */
    fclose(ptr_file);

}
