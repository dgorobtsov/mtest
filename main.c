#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include "config.h"
	
#define MEM_BUS_WIDTH       32UL
#define ALIGNMENT           (MEM_BUS_WIDTH/8)

#define ONE                 0x1UL  
#define ALL_ONES            0xFFFFFFFFUL 
#define ALL_ZEROES          0x0UL
#define PATTERN             0xAAAAAAAA
#define INV_PATTERN         0x55555555

#define UNALIGNED_MASK      (ALIGNMENT - 1)  
#define ALIGNMENT_MASK      (~UNALIGNED_MASK)

typedef enum {
    MEM_TEST_INVALID = 0,
    MEM_TEST_CONST_ONE,
    MEM_TEST_CONST_ZERO,
    MEM_TEST_WALK_ONES, 
    MEM_TEST_WALK_ZEROES,
    MEM_TEST_ADDRESS,          
    MEM_TEST_LIST_TERMINATOR
} mem_test_t;

typedef struct {
    mem_test_t  id; 
    int (*proc) (uint32_t);
    uint32_t    arg;    
} test_case_t;


uint32_t base_addr = DRAM_MAP_ADDR;
uint32_t mem_size = DRAM_SIZE;
uint32_t num_iters = 1;
const char * app_name;

test_case_t test_descr;

char dram_bytes[DRAM_SIZE] __attribute__ ((section(".dram_bss")));

#ifdef BUILD_ASM
#define LINK_QUALFR extern    
#else
#define LINK_QUALFR static
#endif

LINK_QUALFR int run_const_pattern_test(uint32_t);
LINK_QUALFR int run_walking_pattern_test(uint32_t); 
LINK_QUALFR int run_address_test(uint32_t);
static void run_mem_test(const char *);

static const char * const test_descr_strs[] = {
    "Invalid test name",
    "Data bus test: constant one pattern",
    "Data bus test: constant zero pattern",
    "Data bus test: walking ones pattern",
    "Data bus test: walking zeroes pattern",
    "Address bus test"
};    

static void print_usage(const char * app_name)
{
    static const char usage_msg[] = 
        "Usage: %s <test_name> <options>\n"
        "Supported tests:\n"
        "    const_1            %s\n"        
        "    const_0            %s\n"
        "    walk_1s            %s\n"
        "    walk_0s            %s\n"
        "    addr               %s\n"
        "Options:\n"
        "    -a <base_addr>     Base address of memory area (default: %#x)\n"
        "    -s <size>          Size of memory area, bytes (default: %#u)\n"
        "    -n <num_of_iters>  Number of test iterations (default: %#u)\n";

       printf(usage_msg, app_name, 
               test_descr_strs[MEM_TEST_CONST_ONE],
               test_descr_strs[MEM_TEST_CONST_ZERO],
               test_descr_strs[MEM_TEST_WALK_ONES],
               test_descr_strs[MEM_TEST_WALK_ZEROES],
               test_descr_strs[MEM_TEST_ADDRESS],
               base_addr, mem_size, num_iters);         
}


int main(int argc, char* argv[]) 
{ 
	int opt;
    app_name = argv[0];    

	while (-1 != ((opt = getopt( argc, argv, "a:s:n:h")))) {
		switch (opt) {
			case 'a':
	      		base_addr = strtoul(optarg, NULL, 0);
			    break;
			case 's':
			    mem_size = strtoul(optarg, NULL, 0);
			    break;
			case 'n':
			    num_iters = strtoul(optarg, NULL, 0); 
			    break;
            case 'h':
			default:
			    print_usage(app_name);
                return 1;
		}
	}
    if (optind >= argc) {
        print_usage(app_name);
        return 1;
    }
    
    if ( mem_size % ALIGNMENT ) {
        mem_size = mem_size/ALIGNMENT + ALIGNMENT;
    }
    base_addr &= ALIGNMENT_MASK;
    
    run_mem_test(argv[optind]);    

	return 0;
}


static void run_mem_test(const char * test_name) 
{
    uint32_t    pattern;
   
    if ( !strcmp("const_1", test_name) ) {
        test_descr.id = MEM_TEST_CONST_ONE;
        test_descr.proc = run_const_pattern_test;
        test_descr.arg = ALL_ONES;       
    }
    else if ( !strcmp("const_0", test_name) ) {
        test_descr.id = MEM_TEST_CONST_ZERO;
        test_descr.proc = run_const_pattern_test;
        test_descr.arg = ALL_ZEROES;       
    }    
    else if ( !strcmp("walk_1s", test_name) ) {
        test_descr.id = MEM_TEST_WALK_ONES;
        test_descr.proc = run_walking_pattern_test;
        test_descr.arg = 1;  
    }
    else if ( !strcmp("walk_0s", test_name) ) {
        test_descr.id = MEM_TEST_WALK_ZEROES;
        test_descr.proc = run_walking_pattern_test;
        test_descr.arg = 0;  
    }
    else if ( !strcmp("addr", test_name) ) {
        test_descr.id = MEM_TEST_ADDRESS;
        test_descr.proc = run_address_test;
        test_descr.arg = 0;  
    }
    
    if ( NULL != test_descr.proc && MEM_TEST_INVALID != test_descr.id ) {
        uint32_t i = 0;
        printf("Running \'%s\' for %u bytes at base address %#x...\n", 
                test_descr_strs[test_descr.id], mem_size, base_addr);
        for ( i = 0; i < num_iters; i++ ) {
            printf("Iteration %u\n", i + 1);
            if ( 0 != test_descr.proc(test_descr.arg) )
                printf("Failed\n");
            else 
                printf("Passed\n");
        }    
    }
    else {
        printf( "%s\n", test_descr_strs[test_descr.id]);
        print_usage(app_name);     
    }
}


#ifndef BUILD_ASM
static int run_const_pattern_test(uint32_t pattern) 
{
    int rc = 0;
    uint32_t i;
    uint32_t count, value;
    volatile uint32_t * ptr;  
  
    count = mem_size / ALIGNMENT;
    ptr = (uint32_t *)base_addr; 
   
    for ( i = 0; i < count; i++ ) {
        *ptr++ = pattern;                                      
    }
        
    ptr = (uint32_t *)base_addr;
        
    for ( i = 0; i < count; i++ ) {
        if ( pattern != *ptr++ ) { 
            printf("Pattern %#x check failure at address %p, value: %#x \n", pattern, ptr-1 , *(ptr-1));
            rc = 1; 
        }                
    }

    return rc;  
}


static int run_walking_pattern_test(uint32_t pattern) 
{
    int rc = 0;
    uint32_t i, n;
    uint32_t count, value;
    volatile uint32_t * ptr;

    count = mem_size/ALIGNMENT;

    for ( n = 0; n < MEM_BUS_WIDTH; n++) {
        ptr = (uint32_t *) base_addr; 
           
        value = ONE << n;
        if (!pattern)
            value = ALL_ONES ^ value;
            
        for ( i = 0; i < count; i++ ) {
            *ptr++ = value;                                      
        }
       
        ptr = (uint32_t *)base_addr;
        
        for ( i = 0; i < count; i++ ) {
            if ( value != *ptr++ ) { 
                printf("Pattern %#x check failure at address %p, read value: %#x \n", value, ptr-1, *(ptr-1));
                rc = 1; 
            }                
        }
    }
    
    return rc;  
}


static int run_address_test(uint32_t dummy_param)
{          
    uint32_t i, j;
    uint32_t addr_mask = mem_size/ALIGNMENT - 1;  
    volatile uint32_t * ptr = (uint32_t *)base_addr;

    for ( i = 1; (i & addr_mask) != 0; i <<= 1 ) {
        ptr[i] = PATTERN;
    }
   
    ptr[0] = INV_PATTERN;

    for ( i = 1; (i & addr_mask) != 0; i <<= 1 ) {
        if ( ptr[i] != PATTERN ) {
            printf("Address line stuck high: %p\n", &ptr[i]);
            return 1;
        }
    }

    ptr[0] = PATTERN;

    for ( j = 1; (j & addr_mask) != 0; j <<= 1 ) {

        ptr[j] = INV_PATTERN;

		if ( ptr[0] != PATTERN ) {
	        printf("Address line stuck low: %p\n", &ptr[j]); 
            return 1;
		}
        for ( i = 1; (i & addr_mask) != 0; i <<= 1 ) {
            if ( (ptr[i] != PATTERN) && (i != j) ) {
                 printf("Address line short: %p\n", &ptr[j]);
                 return 1;
            }
        }
        ptr[j] = PATTERN;
    }

    return 0;
} 
#endif
