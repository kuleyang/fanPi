/* COMP3000 - Final Project
 * fanPi Kernel Module...Fuck it issa driver.
 * Used to send commands to the device GPIO
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/init.h> 
#include <asm/uaccess.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/string.h>

#define DEVICE_NAME "fanPi"
#define CLASS_NAME "myfanPi" 

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jean-Elie Jean-Gilles <jeaneliejeangilles@cmail.carleton.ca");
MODULE_DESCRIPTION("Raspberry Pi Linux Driver for Fan");
MODULE_VERSION("1.0");

// data to send to userspace
static char fstatus[256] = "OFF";

//Initialize the pin, mang.
static struct gpio fan = {17, GPIOF_OUT_INIT_LOW, "Fan"};
//static unsigned int fan = 17;

//Values for driver numbers and shit.
static int err, majorNumber, numberOpens = 0;
static short statusSize;

static struct class* driverClass = NULL;

static struct device* driverDevice = NULL;

static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

static struct file_operations fops = {
   .open = dev_open,
   .read = dev_read,
   .write = dev_write,
   .release = dev_release,
};

//Initialize the shit
static int __init fanPi_init(void) {
	printk(KERN_INFO "fanPi: initializing\n");
	
	//Give it a major number
	majorNumber = register_chrdev(0, DEVICE_NAME, &fops); 
	if (majorNumber<0) {
		printk(KERN_ALERT "fanPi: failed to allocate major number\n");
		return majorNumber;
	}
	
	printk(KERN_INFO "fanPi: registered with major number %d\n", majorNumber);
	
	//Register the driver class
	driverClass = class_create(THIS_MODULE, CLASS_NAME); 
	if (IS_ERR(driverClass)) {
		unregister_chrdev(majorNumber, DEVICE_NAME);
		printk(KERN_ALERT "fanPi: failed to register device class\n"); 
		return PTR_ERR(driverClass);
	}
	
	printk(KERN_INFO "fanPi: device class registered\n");
   
   // register device driver
	driverDevice = device_create(driverClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME); 

	if (IS_ERR(driverDevice)) {
			// if error, clean up
		class_destroy(driverClass); 
		unregister_chrdev(majorNumber, DEVICE_NAME); 
		printk(KERN_ALERT "fanPi: failed to create device\n"); 
		return PTR_ERR(driverDevice);
	}
	printk(KERN_INFO "fanPi: device class created\n"); 

	
	err = gpio_request_one(fan.gpio, fan.flags, fan.label);
	
	if (err == 0){
		printk(KERN_INFO "fanPi: STARTING FAN");
		statusSize = strlen(fstatus);
	} else {
		printk(KERN_ERR "fanPi: UNABLE TO SET PIN");
		return -ENODEV;
	}

	return 0;
	
}

/*int strcmp(const char* s1, const char* s2)
{
    while(*s1 && (*s1==*s2))
        s1++,s2++;
    return *(const unsigned char*)s1-*(const unsigned char*)s2;
}*/

/* Cleanup */
static void __exit fanPi_exit(void) {
	//Free the GPIO
	gpio_free(fan.gpio);
   // remove device
   device_destroy(driverClass, MKDEV(majorNumber, 0));
   // unregister device class
   class_unregister(driverClass);
   // remove device class
   class_destroy(driverClass);
   // unregister major number
   unregister_chrdev(majorNumber, DEVICE_NAME);
   printk(KERN_INFO "fanPi: closed\n");
}

static int dev_open(struct inode *inodep, struct file *filep) {
	numberOpens++;
	printk(KERN_INFO "fanPi: device opened %d time(s)\n", numberOpens); 
	return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset) { 
	int error_count = 0;
	error_count = copy_to_user(buffer, fstatus, statusSize);	
	if (error_count == 0) {
		printk(KERN_INFO "fanPi: Successfully sent fan state %s to user\n", fstatus);
		return len;
	} else {
		printk(KERN_ERR "fanPi: failed to send fan state %s to user\n", fstatus);
	return -EFAULT;
	} 
}

static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset) {

	if (strcmp(buffer, "ON") == 0){
		strcpy(fstatus, buffer);
		gpio_set_value(fan.gpio,1);
		printk(KERN_INFO "fanPiDEBUG: This is hit!! %s", fstatus);
	} else if (strcmp(buffer, "OFF") == 0) {
		strcpy(fstatus, buffer);
		printk(KERN_INFO "fanPiDEGUG: This is hit!! %s", fstatus);
		gpio_set_value(fan.gpio,0);
	} else if (strcmp(buffer, "AUTO") == 0) {
		strcpy(fstatus, buffer);
	}

		statusSize = strlen(fstatus);
	

	return len;
}
/* Called when device is closed/released.
 * inodep = pointer to inode
 * filep = pointer to a file
 */
static int dev_release(struct inode *inodep, struct file *filep) {
	
	printk(KERN_INFO "fanPi: released\n");
  	return 0;
}
module_init(fanPi_init);
module_exit(fanPi_exit);







