#include <linux/kernel.h>
#include "ept.h"
#include "gmh.h"

ept_pml4e* initialize_eptp () 
{
    int i;

    // EPTP
    pt_eptp ept_ptr = kzalloc(PAGE_SIZE, GFP_KERNEL);
    if (!ept_ptr)
        return NULL;

    // PML4
    pt_pml4e ept_pml4e_ptr = kzalloc(PAGE_SIZE, GFP_KERNEL);
    if (!pept_pml4e) {
        kfree(ept_ptr);
        return NULL;
    }

    // PDPT
    pt_pdpte ept_pdpt_ptr = kzalloc (PAGE_SIZE, GFP_KERNEL);
    if (!ept_pdpt_ptr) {
        kfree(ept_pml4e_ptr);
        kfree(ept_ptr);
        return NULL;
    }

    // Page dir
    pt_pde ept_pde_ptr = kzalloc(PAGE_SIZE, GFP_KERNEL);
    if (!ept_pde_ptr) {
        kfree(ept_pdpt_ptr);
        kfree(ept_pml4e_ptr);
        kfree(ept_ptr);
        return NULL;
    }

    // page table
    pt_pte ept_pte_ptr = kzalloc (PAGE_SIZE, GFP_KERNEL);
    if (!ept_pte_ptr) {
        kfree (ept_pte_ptr);
        kfree(ept_pdpt_ptr);
        kfree(ept_pml4e_ptr);
        kfree(ept_ptr);
        return NULL;
    }

     // Setup PT by allocating two pages Continuously
    // We allocate two pages because we need 1 page for our RIP to start and 1 page for RSP 1 + 1 = 2

    int pages_to_alloc = 10;
    u64 guest_memory = kzalloc (PAGE_SIZE * pages_to_alloc, GFP_KERNEL);
    for (i=0; i<pages_to_alloc, i++)
    {
        ept_pte_ptr[i].fields.AccessedFlag       = 0;
        ept_pte_ptr[i].fields.DirtyFlag          = 0;
        ept_pte_ptr[i].fields.EPTMemoryType      = 6;
        ept_pte_ptr[i].fields.Execute            = 1;
        ept_pte_ptr[i].fields.ExecuteForUserMode = 0;
        ept_pte_ptr[i].fields.IgnorePAT          = 0;
        ept_pte_ptr[i].fields.PhysicalAddress    = (__pa(guest_memory + (i * PAGE_SIZE)) / PAGE_SIZE);
        ept_pte_ptr[i].fields.Read               = 1;
        ept_pte_ptr[i].fields.SuppressVE         = 0;
        ept_pte_ptr[i].fields.Write              = 1;
    }

    // Setting up PDE
    //
    ept_pde_ptr->fields.Accessed           = 0;
    ept_pde_ptr->fields.Execute            = 1;
    ept_pde_ptr->fields.ExecuteForUserMode = 0;
    ept_pde_ptr->fields.Ignored1           = 0;
    ept_pde_ptr->fields.Ignored2           = 0;
    ept_pde_ptr->fields.Ignored3           = 0;
    ept_pde_ptr->fields.PhysicalAddress    = (__pa(EptPt) / PAGE_SIZE);
    ept_pde_ptr->fields.Read               = 1;
    ept_pde_ptr->fields.Reserved1          = 0;
    ept_pde_ptr->fields.Reserved2          = 0;
    ept_pde_ptr->fields.Write              = 1;

        // Setting up PDPTE
    //
    ept_pde_ptrpt->fields.Accessed           = 0;
    ept_pde_ptrpt->fields.Execute            = 1;
    ept_pde_ptrpt->fields.ExecuteForUserMode = 0;
    ept_pde_ptrpt->fields.Ignored1           = 0;
    ept_pde_ptrpt->fields.Ignored2           = 0;
    ept_pde_ptrpt->fields.Ignored3           = 0;
    ept_pde_ptrpt->fields.PhysicalAddress    = (__pa(ept_pde_ptr) / PAGE_SIZE);
    ept_pde_ptrpt->fields.Read               = 1;
    ept_pde_ptrpt->fields.Reserved1          = 0;
    ept_pde_ptrpt->fields.Reserved2          = 0;
    ept_pde_ptrpt->fields.Write              = 1;

        // Setting up PML4E
    //
    ept_pml4e_ptr->fields.Accessed           = 0;
    ept_pml4e_ptr->fields.Execute            = 1;
    ept_pml4e_ptr->fields.ExecuteForUserMode = 0;
    ept_pml4e_ptr->fields.Ignored1           = 0;
    ept_pml4e_ptr->fields.Ignored2           = 0;
    ept_pml4e_ptr->fields.Ignored3           = 0;
    ept_pml4e_ptr->fields.PhysicalAddress    = (__pa(ept_pde_ptrpt) / PAGE_SIZE);
    ept_pml4e_ptr->fields.Read               = 1;
    ept_pml4e_ptr->fields.Reserved1          = 0;
    ept_pml4e_ptr->fields.Reserved2          = 0;
    ept_pml4e_ptr->fields.Write              = 1;

       // Setting up EPTP
    //
    ept_pml4e_ptr->fields.DirtyAndAceessEnabled = 1;
    ept_pml4e_ptr->fields.MemoryType            = 6; // 6 = Write-back (WB)
    ept_pml4e_ptr->fields.PageWalkLength        = 3; // 4 (tables walked) - 1 = 3
    ept_pml4e_ptr->fields.PML4Address           = (__pa(ept_pml4e_ptr) / PAGE_SIZE);
    ept_pml4e_ptr->fields.Reserved1             = 0;
    ept_pml4e_ptr->fields.Reserved2             = 0;

    pr_debug("gmh: EPTP allocated at %llx", __pa(ept_pml4e_ptr));
    return ept_pml4e_ptr;
}
