// @author Jose Carlos Torro Belda
#include <linux/kernel.h>                                                                                                                                                                                                                    
#include <linux/module.h>                                                                                                                                                                                                                    
#include <linux/fs.h>                                                                                                                                                                                                                        
#include <linux/list.h>                                                                                                                                                                                                                      
#include <linux/pci.h>                                                                                                                                                                                                                       
#include <asm/uaccess.h>

#include "cliente.h"

int num;
module_param(num, int, 0);

static int Major;

extern void acumula(int i);
extern int llevamos(void);

static struct file_operations fops;

int init_module(void)
{
    /*Inicialización dinámica*/
    Major = register_chrdev(0, DEVICE_NAME, &fops);

    acumula(num);

    if (Major < 0) 
    {
        printk ("MODULO CLIENTE => Error %d al registrar el dispositivo %s\n", Major, DEVICE_NAME);
        return Major;
    }
    printk("MODULO CLIENTE => Modulo cliente incluido con éxito en %s con Major=%d\n",DEVICE_NAME, Major);
    return Major;

}

void cleanup_module(void)
{   
    int lleva = llevamos();

    printk("MODULO CLIENTE => Modulo cliente. Suma=%d\n",lleva);
    unregister_chrdev(Major, DEVICE_NAME);

    printk("MODULO CLIENTE => Modulo cliente extraído con éxito\n");
}