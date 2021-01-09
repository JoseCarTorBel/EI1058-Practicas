#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>

#define tamanyo_bloque 5000
#define max_bloques 10

#define DEVICE_NAME "servidor"
#define SUCCESS 0

MODULE_LICENSE("GPL");

static char* puntero; // Puntero a la zona de memoria reservada
static int num_bloques=0;


char *reserva_memoria(int i);
void libera_memoria(void);

int init_module(void)
{
    printk("servidor_mem: Insertado modulo servidor_mem \n");
    return 0;
}

void cleanup_module(void)
{
    libera_memoria();
    
    printk("\nservidor_mem: eliminado módulo server_mem.\n");
    printk("servidor_mem: Se han reservado con éxito un total de %d bloques.\n",num_bloques);

}

/**
 * Reserva la memoria, augmenta el numero de bloques que se van a usar
 * y devuelve el puntero.
*/
EXPORT_SYMBOL(reserva_memoria);
char *reserva_memoria(int i)
{
    if(i<0 || i>max_bloques)
    {
       printk("servidor_mem: No se han podido reservar memoria.\n");
       return NULL; // No se puede reservar la memoria
       
    }
    
    puntero = kmalloc(sizeof(tamanyo_bloque*i), GFP_KERNEL);
    printk("\nservidor_mem: Se han reservado %d bloques a partir de la dirección 0x%16lX\n\n",i,(unsigned long)puntero);
    num_bloques+=i;
    return puntero;
}

/**
* Libera la memoria del puntero.
*/
EXPORT_SYMBOL(libera_memoria);
void libera_memoria(void){

    if(puntero != NULL)
    {	    
	printk("servidor_mem: Se libera la memoria reservada a partir de la dirección  0x%16lX\n",(unsigned long)puntero);
        kfree(puntero);
    }
    puntero = NULL;
}