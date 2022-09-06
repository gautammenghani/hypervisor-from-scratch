#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/processor.h>
#include <asm/cpufeatures.h>
#include <asm/cpufeature.h>
#include <asm/msr.h>
struct cpuinfo_x86 cpu;

MODULE_LICENSE("Dual BSD/GPL");

#define gmh_major 117

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

struct file_operations gmh_fops = {
        .read = gmh_read,
        .write = gmh_write,
        .open = gmh_open,
        .release = gmh_release
};

static int __init gmh_init(void)
{
	char *vendor_name;
	int erri, efer;

	if (register_chrdev(gmh_major, "gmh", &gmh_fops) < 0) {
		pr_debug("cannot obtain major number: %d\n", gmh_major);
	}
	cpu = cpu_data(0);
	vendor_name = cpu.x86_vendor_id;
	pr_warn("Vendor name :  %s\n", vendor_name);

	if (!strncmp (vendor_name, "AuthenticAMD", strlen(vendor_name))) {
		pr_warn("SVM supported? :  %s\n", boot_cpu_has(X86_FEATURE_SVM) == 1?"Yes":"No");
		rdmsrl(MSR_EFER, efer);
		if (efer & EFER_SVME)
			pr_warn("EFER_SVME is already set\n");
		else {
			pr_warn("not busy: %d\n", efer);
			//err = svm_hardware_enable();			
			wrmsrl(MSR_EFER, efer | EFER_SVME);
			rdmsrl(MSR_EFER, efer);
		}
	}
	else if (!strncmp (vendor_name, "GenuineIntel", strlen(vendor_name)))
		pr_warn("VMX supported? :  %s\n", boot_cpu_has(X86_FEATURE_VMX) == 1?"Yes":"No");
	else
		pr_warn("Virtualization is not supported\n");
	return 0;
}

static void __exit gmh_exit(void)
{
	pr_warn("exit called\n");
}

module_init(gmh_init);
module_exit(gmh_exit);
