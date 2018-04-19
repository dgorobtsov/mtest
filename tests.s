
.extern base_addr
.extern mem_size
.extern printf

.global run_const_pattern_test
.global run_walking_pattern_test
.global run_address_test

.set MEM_BUS_WIDTH, 32
.set ALIGNMENT, 0x4
.set ALIGNMENT_SHIFT, 0x2
.set PATTERN, 0xAAAAAAAA 
.set INV_PATTERN, 0x55555555 

.section .rodata

err1_str:
    .string "Pattern %#x check failure at address %p, value: %#x\n"
err2_str:
    .string "Address line stuck high: %p\n"
err3_str:
    .string "Address line stuck low: %p\n"
err4_str:
    .string "Address line short: %p\n"

.section .text

# Test of data bus lines wiring.
# Write all ones or zeroes pattern into each location starting from <base_addr>
# up to <base_addr> + <mem_size>. Then read each location in that address range and verify
# if it holds written pattern.  
run_const_pattern_test:
        addi    sp, sp, -24
        sw      ra, 20(sp)
        sw      s1, 16(sp) 
        sw      s2, 12(sp)
        sw      s3,  8(sp) 
        sw      s4,  4(sp)
        sw      s5,  0(sp)
  
        mv      s1, a0
        li      s3, 0       
        lw      s2, mem_size
	    srli	s2, s2, ALIGNMENT_SHIFT
        mv      a0, s2
        lw      a1, base_addr   
2:      sw      s1, 0(a1)
        addi    a1, a1, ALIGNMENT  
        addi    a0, a0, -1
        bnez	a0, 2b

        lw      a2, base_addr
        mv      s4, s2 
3:      lw      a3, 0(a2)
        bne     a3, s1, 5f  
4:      addi    a2, a2, ALIGNMENT
        addi    s4, s4, -1        
        bnez	s4, 3b
        j       6f 

5:      la      a0, err1_str
        mv      a1, s1
        mv      s5, a2     
        jal     ra, printf
        mv      a2, s5
        li      s3, 1
        j       4b            

6:      mv      a0, s3
        lw      ra, 20(sp)
        lw      s1, 16(sp) 
        lw      s2, 12(sp)
        lw      s3,  8(sp)
        lw      s4,  4(sp)
        lw      s5,  0(sp)
        addi    sp, sp, 24
        ret      

# Test data bus lines to find out shorted or stuck low/high.  
# Write pattern that contains a single bit set/unset into each location
# starting from <base_addr> up to <base_addr> + <mem_size>. Then read each location 
# in that address range and verify if it holds written pattern. 
# On the next iteration another bit is set/unset in the next more significant position
# of the word, thus forming walking ones/zeroes pattern.    
run_walking_pattern_test:
        addi    sp, sp, -32
        sw      ra, 28(sp)
        sw      s1, 24(sp) 
        sw      s2, 20(sp)
        sw      s3, 16(sp) 
        sw      s4, 12(sp)
        sw      s5,  8(sp)
        sw      s6,  4(sp)
        sw      s7,  0(sp)

        mv      s1, a0
        li      s3, 0   
        lw      s2, mem_size
	    srli	s2, s2, ALIGNMENT_SHIFT
        li      s6, 0   
2:      lw      a1, base_addr
        li      a2, 1
        sll     s7, a2, s6      
        bnez    s1, 3f
        not     s7, s7 
3:      mv      a0, s2
4:      sw      s7, 0(a1)
        addi    a1, a1, ALIGNMENT  
        addi    a0, a0, -1
        bnez	a0, 4b
        
        lw      a2, base_addr
        mv      s4, s2 
5:      lw      a3, 0(a2)
        bne     a3, s7, 7f  
6:      addi    a2, a2, ALIGNMENT
        addi    s4, s4, -1        
        bnez	s4, 5b
        li      a0, MEM_BUS_WIDTH
        addi    s6, s6, 1
        bgtu    a0, s6, 2b        
        j       8f 

7:      la      a0, err1_str
        mv      a1, s7
        mv      s5, a2     
        jal     ra, printf
        mv      a2, s5
        li      s3, 1
        j       5b            

8:      mv      a0, s3
        lw      ra, 28(sp)
        lw      s1, 24(sp) 
        lw      s2, 20(sp) 
        lw      s3, 16(sp)  
        lw      s4, 12(sp)
        lw      s5,  8(sp)
        lw      s6,  4(sp)
        lw      s7,  0(sp) 
        addi    sp, sp, 32
        ret

# Test address bus lines to find out shorted or stuck high/low.
# Only single line failures can be detected. Each power-of-two address   
# in the range <base_addr>+<mem_size> is written with initial pattern at first.
# Then the first power-of-two address in that range is written with inverted value 
# of initial pattern and all the rest power-of-two locations are read to verify  
# if the initial pattern was modified. The above procedure is performed for 
# each power-of-two location in the specified range to find aliases.   
run_address_test:
        addi    sp, sp, -12
        sw      ra, 8(sp)
        sw      s1, 4(sp) 
        sw      s2, 0(sp)    

        lw      s1, mem_size
	    srli	s1, s1, ALIGNMENT_SHIFT
        addi    s1, s1, -1 
        lw      s2, base_addr
        li      a3, PATTERN
 # Fill all power-of-two locations with initial pattern
        li      a2, 1
        j       2f
1:      slli    a5, a2, ALIGNMENT_SHIFT
        add     a1, s2, a5
        sw      a3, 0(a1)
        slli    a2, a2, 1 
2:      and     a2, s1, a2
        bnez    a2, 1b
# Store ones' complemented value of initial pattern into <base_addr>  
        li      a4, INV_PATTERN 
        sw      a4, 0(s2)
# Read the contents of all power-of-two locations in the range <base_addr>+<mem_size> one by one and 
# check it against initial pattern value to identify the address line that stuck high if any    
        li      a2, 1 
        j       4f
3:      slli    a5, a2, ALIGNMENT_SHIFT
        add     a1, s2, a5
        lw      a0, 0(a1)
        bne     a0, a3, 11f
        slli    a2, a2, 1 
4:      and     a2, s1, a2
        bnez    a2, 3b    

        sw      a3, 0(s2)
# Store inverted value of initial pattern into the next power-of-two location in the range 
# <base_addr>+<mem_size> and check all the rest power-of-two locations to identify address line 
# that is shorted or stuck low. Repeat for each power-of-two location in the specified range.   
        li      a2, 1
        j       9f
5:      slli    a5, a2, ALIGNMENT_SHIFT
        add     a1, s2, a5
# Store inverted value of initial pattern into the next power-of-two location in the range
        sw      a4, 0(a1)
# Read <base_addr> contents and check it against initial pattern to verify if the corresponding 
# address line stuck low     
        lw      a0, 0(s2)
        bne     a0, a3, 12f
# Explore contents of the rest power-of-two locations in the range to verify if the
# corresponding address lines are shorted       
        li      a6, 1
        j       8f 
6:      slli    a5, a6, ALIGNMENT_SHIFT
        add     a7, s2, a5
        lw      a0, 0(a7)

        bne     a0, a3, 13f
7:      slli    a6, a6, 1
8:      and     a6, s1, a6
        bnez    a6, 6b   

        sw      a3, 0(a1)
          
        slli    a2, a2, 1 
9:      and     a2, s1, a2
        bnez    a2, 5b
        li      a0, 0    
        j       15f     

11:     la      a0, err2_str
        j       14f 

12:     la      a0, err3_str
        j       14f
        
13:     beq     a2, a6, 7b    
        la      a0, err4_str
14:     jal     ra, printf
        li      a0, 1        
         
15:     lw      ra, 8(sp)
        lw      s1, 4(sp) 
        lw      s2, 0(sp)
        addi    sp, sp, 12      
        ret
              
