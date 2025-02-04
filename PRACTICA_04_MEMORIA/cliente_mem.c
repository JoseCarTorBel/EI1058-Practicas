#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");

#define DEVICE_NAME "cliente"
#define SUCCESS 0

int numero_bloques, i;
char *memoria;
int TAMANYO_BLOQUE = 5000;


unsigned long CR0, *CR3, CR4, EFER, *direccion_inicio, direccion_inicio_bloque, ps;
unsigned long PS, indexTP1, indexTP2, indexTP3, indexTP4, offset;

/** 
 * PDPTE = 1 Cuando es de 1 GB
 * PDTE = 1 AND PDE = 1 Cuando es de 2 MB
 * PDTE = 0 AND PDE = 0 Cuando es de 4 KB
 */
int PDPTE=0, PDE=0;

unsigned long   flagTablaPaginas01, 
	            flagTablaPaginas02, 
	            flagTablaPaginas03, 
	            flagTablaPaginas04;

unsigned long 	direccionMarcoTP1, 
		        direccionMarcoTP2, 
		        direccionMarcoTP3, 
		        direccionMarcoTP4;
		
unsigned long   *direccionMarcoTP1Virtual, 
	            *direccionMarcoTP2Virtual,
	            *direccionMarcoTP3Virtual, 
	            *direccionMarcoTP4Virtual;

#define mascaraPG  0x80000000   /** BIT 31*/
#define mascaraPAE 0x20         /** BIT 5*/
#define mascaraLME 0x100        /** BIT 8*/

#define mascaraFlags	0x000000fff
#define mascaraMarco  	0xffffff000

#define mascaraDesp4K 0xfffffffffffff800 /*Cuando se trata de pagina 4K*/		      
#define mascaraDesp2M 0xffffffffffe00000/* Cuando se trata de pag de 2M */

#define mascindexTP4 0x1ff000 /*Posiciones de la 20 a la 12 */
#define mascindexTP3 0x3fe00000 /*Posiciones 21 - 30*/
#define mascindexTP2 0x7fc0000000 /*Posiciones 38 - 30*/                       
#define mascindexTP1 0xff8000000000 /* Posiciones 47 - 39 */


module_param(numero_bloques, int, 0);

/**Referencias a los bloques */
extern char* reserva_memoria(int);
extern void libera_memoria(void);


void muestra_info_bloques(unsigned long direccion_inicio_bloque);
void imprime_TP();
int checkPS(unsigned long flag);

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
	direccion_inicio_bloque+=i*TAMANYO_BLOQUE;
	printk("\ncliente_mem: Bloque %d ****************\n\n",i);
	printk("cliente_mem: Dirección física del bloque: 0x%016lX. Dirección virtual del bloque: 0x%016lX \n",
		    (unsigned long)virt_to_phys((volatile void *)direccion_inicio_bloque),
		    direccion_inicio_bloque);
      
	muestra_info_bloques((unsigned long) (direccion_inicio_bloque));
    }
    
    return 0; /*TODO OK*/
}

void cleanup_module(void)
{
    libera_memoria();
    printk("\n\ncliente_mem: Eliminado módulo cliente_mem \n\n");
}

void muestra_info_bloques(unsigned long direccion_inicio_bloque)
{	
    
    /* Mostrar índices Tabla de páginas*/

    indexTP1 = ((unsigned long) direccion_inicio_bloque&mascindexTP1)>>39; /*Desplazamos los 39 bits restantes, indice para la tabla de páginas 1*/
    indexTP2 = ((unsigned long) direccion_inicio_bloque&mascindexTP2)>>30; /*Desplazamos los 30 bits restantes, indice para la tabla de páginas 2*/
    indexTP3 = ((unsigned long) direccion_inicio_bloque&mascindexTP3)>>21; /*Desplazamos los 21 bits restantes, indice para la tabla de páginas 3*/

    /** Para acceder a las tablas de páginas, lo hacemos desde la dirección de inicio
    *   luego vemos su contenido
    */
    /*TABLA DE PÁGINA 1*/    
    flagTablaPaginas01 = (direccion_inicio[indexTP1]&mascaraFlags);
    direccionMarcoTP1 = (direccion_inicio[indexTP1]&mascaraMarco);
    direccionMarcoTP1Virtual = (unsigned long *) phys_to_virt(direccionMarcoTP1);
    printk("cliente_mem: Indice TP nivel 1: %lu(0x%3lX)\n",indexTP1,indexTP1);
    printk("cliente_mem: Flags entrada TP nivel 1:0%lX\n",flagTablaPaginas01);

    /*Tabla de página 2*/
    flagTablaPaginas02 = (direccionMarcoTP1Virtual[indexTP2]&mascaraFlags);
    direccionMarcoTP2 = (direccionMarcoTP1Virtual[indexTP2]&mascaraMarco);
    direccionMarcoTP2Virtual = (unsigned long *) phys_to_virt(direccionMarcoTP2);
    printk("cliente_mem: Indice TP nivel 2: %lu(0x%3lX)\n",indexTP2,indexTP2);
    printk("cliente_mem: Flags entrada TP nivel 2:0%lX\n",flagTablaPaginas02);

    
    PDPTE = checkPS(flagTablaPaginas02);
    if(!PDPTE)/* Pagina de 2MB*/
    {
         /*Tabla de página 3*/
        flagTablaPaginas03 = (direccionMarcoTP2Virtual[indexTP3]&mascaraFlags);
        direccionMarcoTP3 = (direccionMarcoTP2Virtual[indexTP3]&mascaraMarco);
        direccionMarcoTP3Virtual = (unsigned long *) phys_to_virt(direccionMarcoTP3);
        printk("cliente_mem: Indice TP nivel 3: %lu(0x%3lX)\n",indexTP3,indexTP3);
        printk("cliente_mem: Flags entrada TP nivel 3:0%lX\n",flagTablaPaginas03);

        
        PDE = checkPS(flagTablaPaginas03);
        /* Tamaño de tabla de páginas*/
        if(!PDE) /* Página 4 KB*/
        {   
            indexTP4 = ((unsigned long) direccion_inicio_bloque&mascindexTP4) >>12; /*Desplazamos los 12 bits restantes, indice tabla de paginas 4*/
            
            /*Tabla de páginas 4*/
            flagTablaPaginas04       = (direccionMarcoTP3Virtual[indexTP4]&mascaraFlags);
            direccionMarcoTP4        = (direccionMarcoTP3Virtual[indexTP4]&mascaraMarco);
            direccionMarcoTP4Virtual = phys_to_virt(direccionMarcoTP4);
            printk("cliente_mem: Indice TP nivel 4: %lu(0x%3lX)\n",indexTP4,indexTP4);
            printk("cliente_mem: Flags entrada TP nivel 4:0%lX\n",flagTablaPaginas04);       
            printk("cliente_mem: la pagina es de 4 KB\n");
        }
        else{
            printk("cliente_mem: la pagina es de 2 MB\n");
        }       
    }else 
    {
        printk("clciente_mem: la página es de 1 GB\n");
    }

    imprime_TP();
}

/**
 * Mira el PDE y PDTE según corresponda para saber si es de 4KG o 1 GB.
 */
int checkPS(unsigned long flag)
{
    int bit = flag>>7;
    return bit&1;
}


void imprime_TP()
{
    printk("cliente_mem: Dirección física del bloque: 0x%016lX. Dirección virtual del bloque: 0x%016lX \n",
            (unsigned long)virt_to_phys((volatile void *)direccion_inicio_bloque),
            direccion_inicio_bloque);

    printk("cliente_mem: Marco TP nivel 1: Direccion fisica: 0x%016lX. Direccion virtual: 0x%016lX\n",CR3,(unsigned long) direccion_inicio);            
    printk("cliente_mem: Marco TP nivel 2: Direccion fisica: 0x%016lX. Direccion virtual: 0x%016lX\n",direccionMarcoTP1,(unsigned long) direccionMarcoTP1Virtual);
    
    if(!PDPTE)/* Pagina de 2MB*/
    {
        printk("cliente_mem: Marco TP nivel 3: Direccion fisica: 0x%016lX. Direccion virtual: 0x%016lX\n",direccionMarcoTP2,(unsigned long) direccionMarcoTP2Virtual);
        if(!PDE){
            printk("cliente_mem: Marco TP nivel 4: Direccion fisica: 0x%016lX . Direccion virtual: 0x%016lX\n",direccionMarcoTP3,(unsigned long) direccionMarcoTP3Virtual);
            printk("cliente_mem: Direccion del marco: 0x%016lX. Direccion de la página 0x%016lX\n", 
                    direccionMarcoTP4,
                    (unsigned long) direccionMarcoTP4Virtual);   
        
        }else{    
            printk("cliente_mem: Direccion del marco: 0x%016lX. Direccion de la página 0x%016lX\n", 
                    direccionMarcoTP3,
                    (unsigned long) direccionMarcoTP3Virtual);    
        }
    }
}
