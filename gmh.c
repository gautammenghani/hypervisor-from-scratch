#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/processor.h>
#include <asm/cpufeatures.h>
#include <asm/cpufeature.h>
#include <asm/msr.h>
#include <asm/io.h>
#include <asm/ptrace.h>
#include <linux/cpumask.h>
#include <linux/workqueue.h>
#include <linux/device.h>
#include <linux/sched.h>
#include <linux/smp.h>
#include <linux/slab.h>
#include <asm/tlbflush.h>
#include "gmh.h"

struct cpuinfo_x86 cpu;
static struct class *cls;

MODULE_LICENSE("Dual BSD/GPL");

static inline int vmxon(uint64_t phys)
{
	uint8_t ret;

	__asm__ __volatile__ ("vmxon %[pa]; setna %[ret]"
		: [ret]"=rm"(ret)
		: [pa]"m"(phys)
		: "cc", "memory");

	return ret;
}

static inline int vmptrld(uint64_t vmcs_pa)
{
	uint8_t ret;

	__asm__ __volatile__ ("vmptrld %[pa]; setna %[ret]"
		: [ret]"=rm"(ret)
		: [pa]"m"(vmcs_pa)
		: "cc", "memory");

	return ret;
}

bool allocate_vmcs_region(vm_state *guest_state)
{
    u64 physical_buffer;
    unsigned long ia32_vmx_basic_msr;
    uint8_t ret;
    u8 *vmcs_region;

    vmcs_region = kzalloc(VMCS_SIZE, GFP_KERNEL);
    if (!vmcs_region){
        pr_err("GMH: could not allocate memory for vmxon\n");
        return false;
    }
    physical_buffer = __pa(vmcs_region);
    //memset(*physical_buffer, 0, sizeof(*physical_buffer));
    pr_debug("GMH: vmcs virtual address: 0x%llx\n", vmcs_region);
    pr_debug("GMH: vmcs physical buffer address: 0x%llx\n", physical_buffer);
    rdmsrl(MSR_IA32_VMX_BASIC, ia32_vmx_basic_msr);
    *(uint32_t *)vmcs_region = ia32_vmx_basic_msr;
    ret = vmptrld(physical_buffer);
    if (ret){
        pr_err("GMH: vmptrld operation failed: %d\n", ret);
        return false;
    }
    guest_state->vmcs_region = physical_buffer;
    return true;
}

// Memory funcs
bool allocate_vmxon_region(vm_state *guest_state)
{
    u64 physical_buffer;
    int ret;
    unsigned long ia32_vmx_basic_msr;
    u8 *vmxon_region;

    vmxon_region = kzalloc(VMXON_SIZE, GFP_KERNEL);
    if (!vmxon_region){
        pr_err("GMH: could not allocate memory for vmxon\n");
        return false;
    }
    physical_buffer = __pa(vmxon_region);
    //memset(*physical_buffer, 0, sizeof(*physical_buffer));
    pr_debug("GMH: vmxon virtual buffer address: 0x%llx\n", vmxon_region);
    pr_debug("GMH: vmxon physical buffer address: 0x%llx\n", physical_buffer);
    rdmsrl(MSR_IA32_VMX_BASIC, ia32_vmx_basic_msr);
    *(u64 *)vmxon_region = ia32_vmx_basic_msr;
    ret = vmxon(physical_buffer);
    if (ret) {
        pr_err("GMH: vmxon operation failed: %d\n", ret);
        return false;
    }
    guest_state->vmxon_region = physical_buffer;
    return true;
}

// VM FUNCS
static bool run_vmrun(vm_state *guest_state)
{
	u64 *virt_buffer, phys_buffer;
       
	virt_buffer = (u64 *)(kmalloc (sizeof(u8)*4096, GFP_KERNEL));
	if (!virt_buffer) {
		pr_err("Could not allocate memory for VMCB\n");
		return false;
	}
	
	phys_buffer = virt_to_phys(virt_buffer);
	pr_debug("Virtual address: %p\n", virt_buffer);
	pr_debug("Physical address: %lld\n", phys_buffer);
	return true;
}

// FILE OPS FUNCS
int gmh_open(struct inode *inode, struct file *filp)
{
	return 0;
}	

int gmh_release (struct inode *inode, struct file *filp)
{
	return 0;
}

ssize_t gmh_read (struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
	return 0;
}

ssize_t gmh_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
        return 0;
}

static long gmh_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
         switch(cmd) {
                case WR_VALUE:
                                pr_debug("WR_Value\n");
                        	break;
                case RD_VALUE:
                                pr_debug("RD_Value\n");
                        break;
                default:
                        pr_info("Default\n");
                        break;
        }
        return 0;
}

struct file_operations gmh_fops = {
        .read = gmh_read,
        .write = gmh_write,
        .open = gmh_open,
        .release = gmh_release,
	.unlocked_ioctl = gmh_ioctl
};

static void enable_vmxon(void)
{
  cr4_set_bits(X86_CR4_VMXE);
  pr_debug("CR4 - VMXON enabled\n");
}

static void disable_vmxon(void)
{
  cr4_clear_bits(X86_CR4_VMXE);
  pr_debug("CR4 - VMXON disabled\n");
}

static bool vmxon_bit_enabled(void)
{
	int msr_feat_ctrl;
    rdmsrl(MSR_IA32_FEAT_CTL,msr_feat_ctrl);
    pr_debug("MSR_feat_ctrol: %x\n", msr_feat_ctrl);
    if (msr_feat_ctrl & FEAT_CTL_LOCKED){
        if (!(msr_feat_ctrl & FEAT_CTL_VMX_ENABLED_OUTSIDE_SMX)) {
            pr_warn("VMX not enabled in  BIOS\n");
            return false;
        }
    }
    // TODO: Set vmx bit in the msr if not set
    return true;
}

static void vmxoff(void)
{
	__asm__ __volatile__("vmxoff");
}

static long setup_virt_per_cpu(void *data)
{
    int cpu_id;

    cpu_id = smp_processor_id();
    pr_debug("setup_virt() running on cpu: %d\n", cpu_id);
    
    if (!vmxon_bit_enabled())
        return -1;
    enable_vmxon();
    if (!allocate_vmxon_region(&guest_state[cpu_id]))
        pr_warn("cannot allocate vmxon memory\n");
    if (!allocate_vmcs_region(&guest_state[cpu_id]))
        pr_warn("cannot allocate vmcs memory\n");
    
    return 0;
}

static void setup_virt(void)
{
    int cpu, total_cpus;

    total_cpus = num_online_cpus();
    pr_debug("total cpus: %d\n", total_cpus);
    guest_state = kzalloc(sizeof(vm_state) * total_cpus, GFP_KERNEL);
    if (!guest_state){
        pr_err("guest_state buffer could not be allocated\n");
        return;
    }
    for_each_possible_cpu(cpu){
        pr_debug("===== Setup on cpu: %d ========\n", cpu);
        work_on_cpu(cpu, setup_virt_per_cpu, NULL);
    }
}

static long clear_virt_setup_per_cpu(void *data)
{
    int cpu_id; 

    vmxoff();
    disable_vmxon();
    cpu_id = smp_processor_id();
    kfree(__va(guest_state[cpu_id].vmxon_region));
    kfree(__va(guest_state[cpu_id].vmcs_region));
    return 0;
}

static void clear_virt_setup(void)
{
    int cpu, total_cpus;
     
    total_cpus = num_online_cpus();
    
    for_each_possible_cpu(cpu){
        pr_debug("===== Destroy on cpu: %d ========\n", cpu);
        work_on_cpu(cpu, clear_virt_setup_per_cpu, NULL);
    }
    
}

static int __init gmh_init(void)
{
	char *vendor_name;

	cpu = cpu_data(0);
	vendor_name = cpu.x86_vendor_id;
	pr_warn("Vendor name :  %s\n", vendor_name);

	if (!strncmp (vendor_name, "AuthenticAMD", strlen(vendor_name))) {
		pr_warn("SVM supported? :  %s\n", boot_cpu_has(X86_FEATURE_SVM) == 1?"Yes":"No");
	}
	else if (!strncmp (vendor_name, "GenuineIntel", strlen(vendor_name))) {
		pr_warn("VMX supported? :  %s\n", boot_cpu_has(X86_FEATURE_VMX) == 1?"Yes":"No");
        setup_virt();
    }
	else
		pr_warn("Virtualization is not supported\n");
	if (register_chrdev(gmh_major, "gmh", &gmh_fops) < 0) {
		pr_debug("cannot obtain major number: %d\n", gmh_major);
	}
	cls = class_create(THIS_MODULE, "gmh");
	device_create(cls, NULL, MKDEV(gmh_major, 0), NULL, "gmh");
    
	//run_vmrun(NULL);
	return 0;
}

static void __exit gmh_exit(void)
{
    clear_virt_setup();
	device_destroy(cls, MKDEV(gmh_major, 0));
    class_destroy(cls);
	unregister_chrdev(gmh_major, "gmh");
	pr_warn("exit called\n");
}

module_init(gmh_init);
module_exit(gmh_exit);

