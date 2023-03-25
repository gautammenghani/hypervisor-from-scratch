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
#include <linux/device.h>
#include <linux/sched.h>
#include <linux/smp.h>
#include <linux/slab.h>
#include <asm/tlbflush.h>

#define IOC_MAGIC k
#define gmh_major 117

#define WR_VALUE _IOW('a','a',int32_t*)
#define RD_VALUE _IOR('a','b',int32_t*)

typedef struct virtual_machine_state {
	u64 vmcb_region;
} vm_state, *pvm_state;

extern vm_state *g_gueststate;

struct cpuinfo_x86 cpu;
static struct class *cls;

MODULE_LICENSE("Dual BSD/GPL");

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

static void enable_vmxon(void *data)
{
  cr4_set_bits(X86_CR4_VMXE);
  pr_debug("CR4 - VMXON enabled\n");
}

static void disable_vmxon(void *data)
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

static void setup_virt(void)
{
    if (!vmxon_bit_enabled()) {
        pr_warn("VMX is disabled in BOIS\n");
        return;
    }
	on_each_cpu(enable_vmxon, NULL, 1);
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
	on_each_cpu(disable_vmxon, NULL, 1);
	device_destroy(cls, MKDEV(gmh_major, 0));
    class_destroy(cls);
	unregister_chrdev(gmh_major, "gmh");
	pr_warn("exit called\n");
}

module_init(gmh_init);
module_exit(gmh_exit);

