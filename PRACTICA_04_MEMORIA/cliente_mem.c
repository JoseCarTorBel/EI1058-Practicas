#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");

#define DEVICE_NAME "cliente"
#define SUCCESS 0

int numero_bloques, i;
char *memoria;

unsigned long CR0, CR3, CR4, EFER, *direccion_inicio, direccion_inicio_bloque, ps;
unsigned long PS, PML4, directoryPointer, directory, table, offset; 
unsigned long tablaPaginas01, tablaPaginas02, tablaPaginas03, tablaPaginas04;

unsigned long 	direccionMarcoTP1, 
		direccionMarcoTP2, 
		direccionMarcoTP3, 
		direccionMarcoTP4;
		
unsigned long *direccionMarcoTP1Virtual, 
	      *direccionMarcoTP2Virtual,
	      *direccionMarcoTP3Virtual, 
	      *direccionMarcoTP4Virtual;

#define mascaraPG  0x80000000   /** BIT 31*/
#define mascaraPAE 0x20         /** BIT 5*/
#define mascaraLME 0x100        /** BIT 8*/

#define mascaraFlags	0x000000fff
#define mascaraMarco  	0xffffff000

#define mascaraDesp4K 0xfff /*Cuando se trata de pagina 4K*/
#define mascaraDesp2M 0x100000 /* Cuando se trata de pag de 2M */

#define mascaraTabla 0x1ff000 /*Posiciones de la 20 a la 12 */
#define mascaraDirectory 0x3fe00000 /*Posiciones 21 - 29*/
#define mascaraDirectoryPointer 0x7FC0000000 /*Posiciones 38 - 30*/
#define mascaraPML4 0xff8000000000 /* Posiciones 47 - 39 */

#define mascaraPS 0x80

module_param(numero_bloques, int, 0);

/**Referencias a los bloques */
extern char* reserva_memoria(int);
extern void libera_memoria(void);


void muestra_info_bloques(int i);
void imprime_TP();

int init_module(void)
{
    printk("\n\nCliente insertado modulo \n\n");
    /** Llamada a reserva_memoria*/
    memoria = reserva_memoria(numero_bloques);

    CR0 = read_cr0();
    CR3 = read_cr3();
    CR4 = read_cr4();
    /**Se pone la máscara */
    EFER = native_read_msr(0xC0000080);

    printk("cliente_mem: CR0 vale: 0x%016lX",CR0);
    if(CR0 && mascaraPG)
        printk(" y PG está activado\n");
    else
        printk(" y PG está desactivado\n");
    
    printk("cliente_mem: CR4 vale: 0x%016lX",CR4);

    if (CR4 && mascaraPAE)
        printk(" y PAE está activado\n");    
    else
        printk(" y PAE está desactivado\n");

    printk("cliente_mem: EFER vale: 0x%016lX",EFER);
    if (EFER && mascaraLME)
        printk(" y LME está activado\n");    
    else
        printk(" y LME está desactivado\n");
    
    direccion_inicio = phys_to_virt(CR3);
    printk("cliente_mem: CR3 vale: 0x%016lX y su direccion virtual es 0x%016lX",
                    CR3,
                    (unsigned long)direccion_inicio);

    direccion_inicio_bloque = (unsigned long) memoria;

    for (i = 0; i < numero_bloques; i++)
    {
        muestra_info_bloques(i);
    }
    
    return 0; /*TODO OK*/
}

void cleanup_mocule(void)
{
    libera_memoria();
    printk("\n\ncliente_mem: Eliminado módulo cliente_mem \n\n");
    

}

void muestra_info_bloques(int i)
{
    printk("\ncliente_mem: Bloque %d ****************\n\n",i);
    printk("cliente_mem: Dirección física del bloque: 0x%016lX. Dirección virtual del bloque: 0x%016lX \n",
            (unsigned long)virt_to_phys((volatile void *)direccion_inicio_bloque),
            direccion_inicio_bloque);

    /* Mostrar índices Tabla de páginas*/

    PML4 = ((unsigned long) memoria&mascaraPML4)>>39; /*Desplazamos los 39 bits restantes, indice para la tabla de páginas 1*/
    directoryPointer = ((unsigned long) memoria&mascaraDirectoryPointer)>>29; /*Desplazamos los 29 bits restantes, indice para la tabla de páginas 2*/
    directory = ((unsigned long) memoria&mascaraDirectory)>>21; /*Desplazamos los 21 bits restantes, indice para la tabla de páginas 3*/

    /** Para acceder a las tablas de páginas, lo hacemos desde la dirección de inicio
    *   luego vemos su contenido
    */
    /*TABLA DE PÁGINA 1*/    
    tablaPaginas01 = (direccion_inicio[PML4]&mascaraFlags);
    direccionMarcoTP1=(direccion_inicio[PML4]&mascaraMarco)>>12;
    direccionMarcoTP1Virtual = phys_to_virt(direccionMarcoTP1);

    /*Tabla de página 2*/
    tablaPaginas02 = (direccionMarcoTP1Virtual[directoryPointer]&mascaraFlags);
    direccionMarcoTP2 = (direccionMarcoTP1Virtual[directoryPointer]&mascaraMarco)>>12;
    direccionMarcoTP2Virtual = phys_to_virt(direccionMarcoTP2);

    /*Tabla de página 3*/
    tablaPaginas03 = (direccionMarcoTP2Virtual[directory]&mascaraFlags);
    direccionMarcoTP3 = (direccionMarcoTP2Virtual[directory]&mascaraMarco)>>12;
    direccionMarcoTP3Virtual = phys_to_virt(direccionMarcoTP3);

    ps = ((unsigned long)memoria&mascaraPS)>>7;

    /* Tamaño de tabla de páginas*/
    if(ps == 0) /* Página 4 KB*/
    {   
        table = ((unsigned long) memoria&mascaraTabla) >>11; /*Desplazamos los 11 bits restantes, indice tabla de paginas 4*/
        offset = ((unsigned long) memoria&mascaraDesp4K); /*Sacamos el offset*/
        
        /*Tabla de páginas 4*/
        tablaPaginas04 = (direccionMarcoTP3Virtual[table]&mascaraFlags);
        direccionMarcoTP4 = (direccionMarcoTP3Virtual[table]&mascaraMarco)>>12;
        direccionMarcoTP4Virtual = phys_to_virt(direccionMarcoTP4);
    }
    else /* Página 2 MB*/
    {
        offset = ((unsigned long) memoria&mascaraDesp2M);        
    }
    imprime_TP();
 

}

void imprime_TP()
{
  
    printk("cliente_mem: Indice TP nivel 1: %lu(0x%3lX)\n",PML4,PML4);
    printk("cliente_mem: Flags entrada TP nivel 1:0%lX\n",tablaPaginas01);
    
    printk("cliente_mem: Indice TP nivel 2: %lu(0x%3lX)\n",directoryPointer,directoryPointer);
    printk("cliente_mem: Flags entrada TP nivel 2:0%lX\n",tablaPaginas02);
    
    printk("cliente_mem: Indice TP nivel 3: %lu(0x%3lX)\n",directory,directory);
    printk("cliente_mem: Flags entrada TP nivel 3:0%lX\n",tablaPaginas03);
    
    if(ps==0)
    {
        printk("cliente_mem: Indice TP nivel 4: %lu(0x%3lX)\n",table,table);
        printk("cliente_mem: Flags entrada TP nivel 4:0%lX\n",direccionMarcoTP4);       
        printk("cliente_mem: la pagina es de 4 KB");
    } else {     
      printk("cliente_mem: la pagina es de 2 MB");
    }
    
        printk("\n");

    printk("cliente_mem: Dirección física del bloque: 0x%016lX. Dirección virtual del bloque: 0x%016lX \n",
            (unsigned long)virt_to_phys((volatile void *)direccion_inicio_bloque),
            direccion_inicio_bloque);

    printk("cliente_mem: Marco TP nivel 1: Direccion fisica: 0x%016lX . Direccion virtual: 0x%016lX\n",direccionMarcoTP1,(unsigned long) direccionMarcoTP1Virtual);
    printk("cliente_mem: Marco TP nivel 2: Direccion fisica: 0x%016lX . Direccion virtual: 0x%016lX\n",direccionMarcoTP2,(unsigned long) direccionMarcoTP2Virtual);
    printk("cliente_mem: Marco TP nivel 3: Direccion fisica: 0x%016lX . Direccion virtual: 0x%016lX\n",direccionMarcoTP3,(unsigned long) direccionMarcoTP3Virtual);
    if(ps==0)
        printk("cliente_mem: Marco TP nivel 4: Direccion fisica: 0x%016lX . Direccion virtual: 0x%016lX\n",direccionMarcoTP4,(unsigned long) direccionMarcoTP4Virtual);
      
    
}