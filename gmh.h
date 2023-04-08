#ifndef __KVM_X86_VMX_H
#define __KVM_X86_VMX_H

#define VMCS_SIZE   4096
#define VMXON_SIZE  4096
#define ALIGNMENT_PAGE_SIZE 4096

#define IOC_MAGIC k
#define gmh_major 117

#define WR_VALUE _IOW('a','a',int32_t*)
#define RD_VALUE _IOR('a','b',int32_t*)

typedef struct virtual_machine_state {
	u64 vmxon_region;
    u64 vmcs_region;
} vm_state, *pvm_state;

vm_state *guest_state;

bool allocate_vmxon_region(vm_state guest_state);
bool allocate_vmcs_region(vm_state guest_state);
#endif
