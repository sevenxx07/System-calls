#include "ec.h"
#include "ptab.h"
#include "string.h"
#include "stdio.h"
 
 
typedef enum {
    sys_print      = 1,
    sys_sum        = 2,
    sys_break      = 3,
    sys_thr_create = 4,
    sys_thr_yield  = 5,
} Syscall_numbers;
 
void Ec::syscall_handler (uint8 a)
{
    // Get access to registers stored during entering the system - see
    // entry_sysenter in entry.S
    Sys_regs * r = current->sys_regs();
    Syscall_numbers number = static_cast<Syscall_numbers> (a);
 
    switch (number) {
        case sys_print: {
            char *data = reinterpret_cast<char*>(r->esi);
            unsigned len = r->edi;
            for (unsigned i = 0; i < len; i++)
                printf("%c", data[i]);
            break;
        }
        case sys_sum: {
            // Naprosto nepotřebné systémové volání na sečtení dvou čísel
            int first_number = r->esi;
            int second_number = r->edi;
            r->eax = first_number + second_number;
            break;
        }
        case sys_break: {
            mword adress = r->esi;
            mword size_of_page = 0x1000;
            printf("current %lx\n", break_current);
            //break called just for checking the current adress 
            if(adress == 0){
                r->eax = break_current;
                break;
            }
            //cant go over heap or in program space
            if(adress > 0xBFFFF000 || adress < break_min){
                r->eax = 0;
                break;
            }
            int fault = 0;
            //make bigger heap
            if(adress > break_current){
                mword max = break_current;
                mword plus_page = 0;
                if(break_current % size_of_page != 0){
                    plus_page = break_current % size_of_page;
                    max = max - plus_page + size_of_page;
                }
                //size of pages we need to allocate
                unsigned int size = 0;
                mword my_adress = adress;
                plus_page = 0;
                if(adress % size_of_page != 0){
                    plus_page = adress % size_of_page;
                    printf("plus page %lx\n", plus_page);
                    my_adress = my_adress - plus_page;
                }
                my_adress = my_adress - max;
                size = my_adress/size_of_page;
                if(plus_page != 0){
                    if(adress - break_current < size_of_page && break_current %size_of_page != 0){
                        size = 0;
                        plus_page = 0;
                    }else{
                        size++;
                    }
                }
                printf("how many pages do I need: %x\n", size);
                unsigned int i;
                my_adress = max; 
                void *b;
                //page to page
                for(i = 0; i < size; i++){
                    //allocate virtual memmory in kernel space
                    b = Kalloc::allocator.alloc_page(1, Kalloc::FILL_0);
                    if(b == NULL){
                        fault = 1;
                        break;
                    }
                    //virtual address from kernel space to physical address
                    mword physaddr = Kalloc::virt2phys(b);
                    //map physical address back to user space
                    if(!Ptab::insert_mapping(my_adress, physaddr, Ptab::PRESENT | Ptab::USER | Ptab::RW)){
                        fault = 1;
                        break;
                    }
                    my_adress = my_adress + size_of_page;
                }
                if(fault){
                    //deallocate everything to i
                    for(unsigned int j = 0; j < i; j++){
                        my_adress = my_adress - size_of_page;
                        mword physaddr = Ptab::get_mapping(my_adress);
                        physaddr = (physaddr >> 12) << 12;
                        if(!Ptab::insert_mapping(my_adress, physaddr, 0)){
                            fault = 1;
                            break;
                        }
                        void *p = Kalloc::phys2virt(physaddr);
                        printf("deallocated (caused by fault in allocation): %lx\n", my_adress);
                        Kalloc::allocator.free_page(p);
                    }
                    printf("Fault by allocating memmory %lx\n", adress);
                    r->eax = 0;
                    break;
                }
            //make smaller heap
            }else if(adress < break_current){
                mword max = break_current;
                mword plus_page = 0;
                if(break_current % size_of_page != 0){
                    plus_page = break_current % size_of_page;
                    max = max - plus_page + size_of_page;
                }
                unsigned int size = 0;
                mword my_adress = adress;
                plus_page = 0;
                if(adress % size_of_page != 0){
                    plus_page = adress % size_of_page;
                    my_adress = my_adress - plus_page;
                    printf("plus page %lx\n", plus_page);
                    my_adress = my_adress + size_of_page;
                }
                my_adress = max - my_adress;
                size = my_adress/size_of_page;
                my_adress = max - size_of_page;
                printf("how many pages do I need: %x\n", size);
                unsigned int i;
                for(i = 0; i < size; i++){
                    mword physaddr = Ptab::get_mapping(my_adress);
                    physaddr = (physaddr >> 12) << 12;
                    if(!Ptab::insert_mapping(my_adress, physaddr, 0)){
                        fault = 1;
                        break;
                    }
                    void *b = Kalloc::phys2virt(physaddr);
                    printf("deallocated: %lx\n", my_adress);
                    Kalloc::allocator.free_page(b);
                    my_adress = my_adress - size_of_page;
                }
                if(fault){
                    r->eax = 0;
                    break;
                }
                if(plus_page != 0){
                    mword physaddr = Ptab::get_mapping(my_adress);
                    physaddr = (physaddr >> 12) << 12;
                    void *b = Kalloc::phys2virt(physaddr);
                    printf("zeros from: %p\n", b);
                    b = static_cast<char*>(b) + plus_page;
                    memset(b, 0, size_of_page - plus_page);
 
                }
            }
            //fault occurs - should not end here
             
            if(fault){
                r->eax = 0;
            }else{
                r->eax = break_current;
                break_current = adress;
            }
            break;
        }
        default:
            printf ("unknown syscall %d\n", number);
            break;
    };
 
    ret_user_sysexit();
}
