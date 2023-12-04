#include <asm/io.h>
#include <linux/kernel.h>
#include <linux/mm_types.h>
#include <linux/module.h>
#include <linux/pid.h>
#include <linux/pid_namespace.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/version.h>
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Linux module for OS Lab2");
MODULE_VERSION("1.0");

#define PROCFS_MAX_SIZE 200
#define procfs_name "lab2"

static struct proc_dir_entry *our_proc_file;
static char procfs_buffer[PROCFS_MAX_SIZE] = {0};
static unsigned long procfs_buffer_size = 0;

static ssize_t procfs_read(struct file *file, char __user *buffer,
                           size_t length, loff_t *offset) {
  printk(KERN_NOTICE "procfs_read: procfs_buffer=%s\n", procfs_buffer);
  static int finished = 0;
  procfs_buffer_size = PROCFS_MAX_SIZE - 1;
  if (finished) {
    pr_debug("procfs_read: END\n");
    finished = 0;
    return 0;
  }
  finished = 1;
  pid_t pid = 0;
  char number[PROCFS_MAX_SIZE] = {0};
  strcpy(number, procfs_buffer);
  kstrtol(number, 10, (long *)&pid);
  struct task_struct *task = pid_task(find_vpid(pid), PIDTYPE_PID);
  struct cpu_itimer tmr = {.expires = 0};
  if (task != NULL) {
    printk(KERN_NOTICE "procfs_read: task_struct is not null!\n");
    if (task->signal != NULL) {
      printk(KERN_NOTICE "procfs_read: task_struct->signal is not null!\n");
      tmr = task->signal->it[0];
    } else {
      printk(KERN_NOTICE "procfs_read: task_struct->signal is null!\n");
      return 0;
    }
  } else {
    printk(KERN_NOTICE "procfs_read: task_struct is null");
    return 0;
  }
  char str[100] = {0};
  snprintf(str, 100, "\ncpu_itime: expires=%llu, incr=%llu\n", tmr.expires,
           tmr.incr);
  strcat(procfs_buffer, str);
  printk(KERN_NOTICE "%s", procfs_buffer);
  if (copy_to_user(buffer, procfs_buffer, procfs_buffer_size))
    return -EFAULT;
  printk(KERN_NOTICE "procfs_read: read %lu bytes\n", procfs_buffer_size);
  return procfs_buffer_size;
}

static int procfs_open(struct inode *inode, struct file *file) {
  try_module_get(THIS_MODULE);
  return 0;
}
static int procfs_close(struct inode *inode, struct file *file) {
  module_put(THIS_MODULE);
  return 0;
}

static ssize_t procfs_write(struct file *file, const char __user *buffer,
                            size_t len, loff_t *off) {
  memset(procfs_buffer, 0,PROCFS_MAX_SIZE);
  procfs_buffer_size = len;
  if (procfs_buffer_size > PROCFS_MAX_SIZE)
    procfs_buffer_size = PROCFS_MAX_SIZE;
  if (copy_from_user(procfs_buffer, buffer, procfs_buffer_size))
    return -EFAULT;
  procfs_buffer[procfs_buffer_size & (PROCFS_MAX_SIZE - 1)] = '\0';
  pr_info("procfile write %s\n", procfs_buffer);
  return procfs_buffer_size;
}

static const struct proc_ops proc_file_fops = {.proc_read = procfs_read,
                                               .proc_write = procfs_write,
                                               .proc_open = procfs_open,
                                               .proc_release = procfs_close};

static int __init procfs_init(void) {
  our_proc_file = proc_create(procfs_name, 0777, NULL, &proc_file_fops);
  if (NULL == our_proc_file) {
    proc_remove(our_proc_file);
    pr_alert("Error:Could not initialize /proc/%s\n", procfs_name);
    return -ENOMEM;
  }
  proc_set_user(our_proc_file, GLOBAL_ROOT_UID, GLOBAL_ROOT_GID);
  pr_info("/proc/%s created\n", procfs_name);
  return 0;
}
static void __exit procfs_exit(void) {
  proc_remove(our_proc_file);
  pr_info("/proc/%s removed\n", procfs_name);
}
module_init(procfs_init);
module_exit(procfs_exit);
