#include<stdio.h>
#include <stdlib.h>
#include<string.h>
#include <stdlib.h>
#include<ctype.h>
#include "cabeceras.h"

#define LONGITUD_COMANDO 100
#define NUM_COMANDOS 8

void PrintBytemaps(EXT_BYTE_MAPS *ext_bytemaps);
int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2);
void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup);
int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, 
              char *nombre);
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos);
void Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, 
              char *nombreantiguo, char *nombrenuevo);
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, 
             EXT_DATOS *memdatos, char *nombre);
int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
           EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
           char *nombre,  FILE *fich);
int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
           EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
           EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino,  FILE *fich);
void GrabarDirectorio(EXT_ENTRADA_DIR *directorio, FILE *fich);
void GrabarInodos(EXT_BLQ_INODOS *inodos, FILE *fich);
void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich);
void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich);
void GrabarDatos(EXT_DATOS *memdatos, FILE *fich);

char *leeLinea(int tam);

char * listaComandos[NUM_COMANDOS] = {"bytemaps", "copy", "dir", "info", "imprimir", "rename", "remove", "salir"};

int main()
{
	char* comando;
	char* orden = (char *)malloc(LONGITUD_COMANDO);
	char* argumento1 = (char *)malloc(LONGITUD_COMANDO);
	char* argumento2 = (char *)malloc(LONGITUD_COMANDO);
	 
	int i,j;
	unsigned long int m;
   EXT_SIMPLE_SUPERBLOCK ext_superblock;
   EXT_BYTE_MAPS ext_bytemaps;
   EXT_BLQ_INODOS ext_blq_inodos;
   EXT_ENTRADA_DIR directorio[MAX_FICHEROS];
   EXT_DATOS memdatos[MAX_BLOQUES_DATOS];
   EXT_DATOS datosfich[MAX_BLOQUES_PARTICION];
   int entradadir;
   int grabardatos = 0;
   FILE *fent;
     

   fent = fopen("particion.bin","r+b");
   fread(&datosfich, SIZE_BLOQUE, MAX_BLOQUES_PARTICION, fent);    
     
     
   memcpy(&ext_superblock,(EXT_SIMPLE_SUPERBLOCK *)&datosfich[0], SIZE_BLOQUE);
   memcpy(&directorio,(EXT_ENTRADA_DIR *)&datosfich[3], SIZE_BLOQUE);
   memcpy(&ext_bytemaps,(EXT_BLQ_INODOS *)&datosfich[1], SIZE_BLOQUE);
   memcpy(&ext_blq_inodos,(EXT_BLQ_INODOS *)&datosfich[2], SIZE_BLOQUE);
   memcpy(&memdatos,(EXT_DATOS *)&datosfich[4],MAX_BLOQUES_DATOS*SIZE_BLOQUE);

     
   // Buce de tratamiento de comandos
   for (;;)
   {
		do 
      {
		   printf (">> ");
		   fflush(stdin);
		   comando = leeLinea(LONGITUD_COMANDO);

         orden = strtok(comando, " ");
         argumento1 = strtok(NULL, " ");
         argumento2 = strtok(NULL, " ");   //Separa la orden y los argumentos


		} 
      while (ComprobarComando(comando,orden,argumento1,argumento2) !=0);

      

	   if (strcmp(orden,"dir")==0) 
      {
         Directorio(directorio,&ext_blq_inodos);
      }

      else if (strcmp(orden,"bytemaps")==0) 
      {
         PrintBytemaps(&ext_bytemaps);
      }

      else if (strcmp(orden,"info")==0) 
      {
         LeeSuperBloque(&ext_superblock);
      }
      else if (strcmp(orden, "rename")==0){
         Renombrar(directorio, &ext_blq_inodos, argumento1, argumento2);
         grabardatos = 1;
      }
      else if (strcmp(orden, "imprimir") == 0) 
      {
         
        Imprimir(directorio, &ext_blq_inodos, memdatos, argumento1);
      }
      else if(strcmp(orden, "copy")==0)
      {
         Copiar(directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, memdatos, argumento1, argumento2, fent);
         grabardatos = 1;
      }

      else if(strcmp(orden, "remove")==0)
      {
         Borrar(directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, argumento1, fent);
         grabardatos = 1;
      }

      
         
      //Si el comando es salir se habrán escrito todos los metadatos
      //faltan los datos y cerrar
      if (strcmp(orden,"salir")==0)
      {
         fclose(fent);
         if (grabardatos)
         {
            fent = fopen("particion.bin","w+b");
            GrabarSuperBloque(&ext_superblock, fent);
            GrabarDirectorio(directorio, fent);
            GrabarByteMaps(&ext_bytemaps, fent);
            GrabarInodos(&ext_blq_inodos, fent);
            GrabarDatos(memdatos, fent);
         }

         
         return 0;
      }
   }
}

//Para saber si una palabra esta en una lista
int palabraEnLista(char *palabra, char **lista, int tamLista)
{
   int res = 0;   //Si es 0, no está; si es 1, está
   for(int i = 0; i < tamLista; i++)
   {
      if(strcmp(palabra, lista[i]) == 0)
      {
         res = 1;
      }
   }

   return res;
}

char *leeLinea(int tam)
{
   int i = 0;
   char c;
   char *res = (char *)malloc(tam);
   do
   {
      c = getchar();
      if(c != '\n')
      {
         res[i++] = c;
      }
      

   }
   while(c != '\n' && i < tam);

   res[i] = '\0';

   return res;

}

void PrintBytemaps(EXT_BYTE_MAPS *ext_bytemaps)
{
   printf("Bytemap de bloques [1-25]:\n");
   for(int i = 0; i < 25; i++)
   {
      printf("%d ", (*ext_bytemaps).bmap_bloques[i] );
   }
   printf("\n");
   printf("Bytemap de inodos:\n");
   for(int i = 0; i < sizeof((*ext_bytemaps).bmap_inodos); i++)
   {
      printf("%d ", (*ext_bytemaps).bmap_inodos[i] );
   }
   printf("\n");
}

int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2)
{
   int res = 1;  //Si es 0, es válido. Si no, es invalido
   if(palabraEnLista(orden, listaComandos, NUM_COMANDOS) == 1)
   {
      res = 0;
   }

   else
   {
      printf("ERROR: Comando ilegal [bytemaps, copy, dir, info, imprimir, rename, remove, salir]\n");
   }


   return res;
}

void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup)
{
   printf("Bloque %d Bytes\n", (*psup).s_block_size);
   printf("inodos particion = %d\n", (*psup).s_inodes_count);
   printf("inodos libres = %d\n", (*psup).s_free_inodes_count);
   printf("Bloques particion = %d\n", (*psup).s_blocks_count);
   printf("Bloques libres = %d\n", (*psup).s_free_blocks_count);
   printf("Primer bloque de datos = %d\n", (*psup).s_first_data_block);
}

int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombre)  //Devuelve 1 si existe y 0 si no
{
   int encontrado = -1;

   //Recorre los nombres hasta que lo encuentre. Si no lo encuentra, se queda en -1
   for(int i = 0; i < sizeof(directorio); i++)
   {
      if(strcmp(directorio[i].dir_nfich, nombre) == 0)
      {
         encontrado = i;
      }
   }

   return encontrado;
}

void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos)
{


   printf("Ficheros :\n");

    
    
   for (int i = 1; i < MAX_FICHEROS; i++) 
   {
      if (directorio[i].dir_inodo != NULL_INODO)  
      {
         EXT_SIMPLE_INODE *inodo = &inodos->blq_inodos[directorio[i].dir_inodo];
         
         printf("Nombre: %s\t", directorio[i].dir_nfich);
         printf("Tamaño: %d\t", inodo->size_fichero);    
         printf("Inodo: %d\t", directorio[i].dir_inodo);  
        

         printf("Bloques: ");

         //Recorre los bloques
         for (int j = 0; j < MAX_NUMS_BLOQUE_INODO; j++) 
         {
            if (inodo->i_nbloque[j] != NULL_BLOQUE)
            {
               printf("%d ", inodo->i_nbloque[j]); 
            }
            
         }
         printf("\n");
      }
   }
}

void Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombreantiguo, char *nombrenuevo)
{
   int posicionNombreAntiguo = -1, posicionNombreNuevo = -1;
   for(int i = 0; (i < MAX_FICHEROS); i++)
   {
      posicionNombreAntiguo = BuscaFich(directorio, inodos, nombreantiguo);
      posicionNombreNuevo = BuscaFich(directorio, inodos, nombrenuevo);
   }
   if(posicionNombreAntiguo < 0)
   {
      printf("ERROR: Fichero %s no encontrado\n", nombreantiguo);
   }
   else if(posicionNombreNuevo >= 0)
   {
      printf("ERROR: El fichero %s ya existe\n", nombrenuevo);
   }
   else
   {
      memcpy(directorio[posicionNombreAntiguo].dir_nfich, nombrenuevo, LEN_NFICH); //Cambio de nombre exitoso
      printf("El nombre de %s se ha cambiado a %s con éxito.\n", nombreantiguo, nombrenuevo);
   }
}

int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino, FILE *fich)
{
   int salir = 0;
   int indiceOrigen = BuscaFich(directorio, inodos, nombreorigen);
   if (indiceOrigen == -1) 
   {
      printf("ERROR: Fichero %s no encontrado\n", nombreorigen);
      return -1; //Si el fichero origen no existe, sale y devuelve -1
   }


   if (BuscaFich(directorio, inodos, nombredestino) >= 0) 
   {
      printf("ERROR: El fichero %s ya existe\n", nombredestino);
      return -1;  //Si el fichero destino existe, sale y devuelve -1
   }

   
   int nuevo_inodo = -1;   //Para buscar un nuevo inodo

   //Por si no hay suficientes inodos
   if(ext_superblock->s_free_inodes_count == 0)
   {
      printf("ERROR: No hay inodos libres.\n");
      return -1; 
   }

   for (int i = 0; i < MAX_INODOS && nuevo_inodo == -1; i++)   
   {
      if (ext_bytemaps->bmap_inodos[i] == 0) 
      {
         nuevo_inodo = i;
         ext_bytemaps->bmap_inodos[i] = 1;   //Se actualiza el bytemap
         ext_superblock->s_free_inodes_count--;
      }
   }


   //Inodo del fichero origen
   EXT_SIMPLE_INODE *inodo_origen = &inodos->blq_inodos[directorio[indiceOrigen].dir_inodo];

   // Creamos un nuevo inodo para el fichero destino y copiamos el inodo del fichero destino
   EXT_SIMPLE_INODE *inodo_destino = &inodos->blq_inodos[nuevo_inodo];
   memcpy(inodo_destino, inodo_origen, sizeof(EXT_SIMPLE_INODE)); 
   inodo_destino->size_fichero = inodo_origen->size_fichero;

   //Copiamos también los bloques
   int bloques_necesarios = (inodo_origen->size_fichero + SIZE_BLOQUE - 1)/SIZE_BLOQUE;
   int bloques_asignados = 0;
   for (int i = 0; i < MAX_NUMS_BLOQUE_INODO && salir == 0; i++) 
   {
      if (bloques_asignados >= bloques_necesarios) 
      {
         inodo_destino->i_nbloque[i] = NULL_BLOQUE; 
         continue;
         //Cuando haya suficientes bloques, los demás los ponemos como null
      }

      for (int j = 0; j < MAX_BLOQUES_DATOS && salir == 0; j++) 
      {
         if (ext_bytemaps->bmap_bloques[j] == 0) 
         {
            ext_bytemaps->bmap_bloques[j] = 1; 
            ext_superblock->s_free_blocks_count--;
            inodo_destino->i_nbloque[i] = j;

           
            memcpy(memdatos[j].dato, memdatos[inodo_origen->i_nbloque[bloques_asignados]].dato, SIZE_BLOQUE);
            bloques_asignados++;
            salir = 1;

            //Si el bloque está libre, se lo asignamos y copiamos la información
         }
      }

      salir = 0;
   }
   if (bloques_asignados < bloques_necesarios)   //Si no hay bloques suficientes
   {
      printf("ERROR: No hay bloques suficientes para copiar el fichero\n");
      return -1;
   }

   //Buscamos una entrada libre en el directorio para poner el nuevo inodo
   for (int i = 0; i < MAX_FICHEROS && salir == 0; i++) 
   {
      if (directorio[i].dir_inodo == NULL_INODO) 
      {
         directorio[i].dir_inodo = nuevo_inodo;
         strncpy(directorio[i].dir_nfich, nombredestino, LEN_NFICH);
         salir = 1;
      }
   }
   salir = 0;

   printf("Fichero %s copiado a %s correctamente.\n", nombreorigen, nombredestino);
   return 0;
}

int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre)
{

   int indice = BuscaFich(directorio, inodos, nombre);   //Para ver si existe el fichero

   if(indice != -1)
   {

      EXT_SIMPLE_INODE inodo = inodos->blq_inodos[directorio[indice].dir_inodo];

      for(int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++)
      {

         if (inodo.i_nbloque[i] != NULL_BLOQUE)
         {
            printf("%s", memdatos[inodo.i_nbloque[i]].dato);  //Imprime los contenidos de los bloques
         }
         
      }

      printf("\n");
   }

   else
   {
      printf("Fichero no encontrado: %s.\n", nombre);
   }

   return 0;
}



int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, char *nombre, FILE *fich)

{
   int inodo_a_eliminar = -1, salir = 0;

    // Buscar la entrada del directorio correspondiente al nombre del fichero
    for (int i = 0; i < MAX_FICHEROS && !salir; i++) {
        if (strcmp(directorio[i].dir_nfich, nombre) == 0) {
            inodo_a_eliminar = directorio[i].dir_inodo;
            salir = 1;
        }
    }

    // Si no se encuentra el fichero, devolver un error
    if (inodo_a_eliminar == -1) {
        printf("Error: fichero no encontrado\n");
        return -1;  // Error: archivo no encontrado
    }

    //Liberar el inodo correspondiente
    if (inodo_a_eliminar != NULL_INODO) {
        EXT_SIMPLE_INODE *inodo = &inodos->blq_inodos[inodo_a_eliminar];

        // Marcar el inodo como libre en el bytemap de inodos
        ext_bytemaps->bmap_inodos[inodo_a_eliminar] = 0;

        // Establecer el tamaño del fichero a 0
        inodo->size_fichero = 0;

        // Marcar los punteros de bloques como libres (FFFFH)
        for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {
            inodo->i_nbloque[i] = NULL_BLOQUE;
        }

        //Liberar los bloques de datos
        for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {
            if (inodo->i_nbloque[i] != NULL_BLOQUE) {
                // Marcar el bloque como libre en el bytemap de bloques
                ext_bytemaps->bmap_bloques[inodo->i_nbloque[i]] = 0;
            }
        }
    }

    //Eliminar la entrada del directorio
    salir =0;
    for (int i = 0; i < MAX_FICHEROS && !salir; i++) {
        if (directorio[i].dir_inodo == inodo_a_eliminar) {
            // Poner el nombre vacío y el número de inodo a NULL_INODO
            memset(directorio[i].dir_nfich, 0, LEN_NFICH);
            directorio[i].dir_inodo = NULL_INODO;
            salir =1;
        }
    }

    //Grabar los cambios en el fichero
    //Grabarinodosydirectorio(directorio, inodos, fich);
    //GrabarByteMaps(ext_bytemaps, fich);
    //GrabarSuperBloque(ext_superblock, fich);
    return 0;  // Éxito
}

//Funciones para grabar los datos en la particion una vez se termina de usar

void GrabarDirectorio(EXT_ENTRADA_DIR *directorio, FILE *fich) 
{
   fseek(fich, SIZE_BLOQUE * 3, SEEK_SET); 
   fwrite(directorio, SIZE_BLOQUE, 1, fich);
}

void GrabarInodos(EXT_BLQ_INODOS *inodos, FILE *fich) 
{
   fseek(fich, SIZE_BLOQUE * 2, SEEK_SET); 
   fwrite(inodos, SIZE_BLOQUE, 1, fich);
}

void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich) 
{
   fseek(fich, SIZE_BLOQUE * 1, SEEK_SET); 
   fwrite(ext_bytemaps, SIZE_BLOQUE, 1, fich);
}

void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich) 
{
   fwrite(ext_superblock, SIZE_BLOQUE, 1, fich);
}

void GrabarDatos(EXT_DATOS *memdatos, FILE *fich) 
{
   fseek(fich, SIZE_BLOQUE * 4, SEEK_SET);
   fwrite(memdatos, SIZE_BLOQUE, MAX_BLOQUES_DATOS, fich);
}

