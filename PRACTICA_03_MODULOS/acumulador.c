// @author Jose Carlos Torro Belda

#include <linux/kernel.h>                                                                                            
#include <linux/module.h>                                                                                            
#include <linux/fs.h>                                                                                                
#include <linux/list.h>                                                                                              
#include <linux/pci.h>                                                                                               
#include <asm/uaccess.h>                                                                                             
                                                                                                                     
#include "acumulador.h"                                                                                              
                                                                                                                     
static int Major;                                                                                                    
int acum=0;;

EXPORT_SYMBOL(acumula);
EXPORT_SYMBOL(llevamos);

static struct file_operations fops;

int init_module(void)
{
    /*Inicialización dinámica*/
    Major = register_chrdev(0, DEVICE_NAME, &fops);
    if (Major < 0) 
    {
        printk ("MODULO ACUMULADOR => Error %d al registrar el dispositivo %s\n", Major, DEVICE_NAME);
        return Major;
    }
    printk("MODULO ACUMULADOR => Modulo acumulador incluido con éxito en %s con Major=%d\n",DEVICE_NAME, Major);
    return Major;
}

void cleanup_module(void)
{
    printk("MODULO ACUMULADOR => Modulo acumulador. Suma=%d\n",acum);
    unregister_chrdev(Major, DEVICE_NAME);
    printk("MODULO ACUMULADOR => Modulo acumulador extraído con éxito\n");
}

void acumula(int i)
{
    acum += i;
}

int llevamos(void)
{
    printk("MODULO ACUMULADOR => Valor acumulado %d\n", acum);
    return acum;
}
