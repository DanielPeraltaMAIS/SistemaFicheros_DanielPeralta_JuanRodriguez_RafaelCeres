#include<stdio.h>
#include <stdlib.h>
#include<string.h>
#include <stdlib.h>
#include<ctype.h>
#include "cabeceras.h"

#define LONGITUD_COMANDO 100

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
void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich);
void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich);
void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich);
void GrabarDatos(EXT_DATOS *memdatos, FILE *fich);

char *leeLinea(int tam);

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
   int grabardatos;
   FILE *fent;
     
     // Lectura del fichero completo de una sola vez
     fent = fopen("particion.bin","rb+");
     fread(&datosfich, SIZE_BLOQUE, MAX_BLOQUES_PARTICION, fent);    
     
     
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
         continue;
      }

      else if (strcmp(orden,"bytemaps")==0) 
      {
         PrintBytemaps(&ext_bytemaps);
         continue;
      }

      else if (strcmp(orden,"info")==0) 
      {
         LeeSuperBloque(&ext_superblock);
         continue;
      }
      else if (strcmp(orden, "rename")==0){
         Renombrar(directorio, &ext_blq_inodos, argumento1, argumento2);
         continue;
      }
      else if (strcmp(orden, "imprimir") == 0) 
      {
         
        Imprimir(directorio, &ext_blq_inodos, memdatos, argumento1);
      }
      else if(strcmp(orden, "copy")==0)
      {
         Copiar(directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, memdatos, argumento1, argumento2, fent);
      }

      else if(strcmp(orden, "remove")==0)
      {
         Borrar(directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, argumento1, fent);
      }
      // Escritura de metadatos en comandos rename, remove, copy     
      //Grabarinodosydirectorio(&directorio,&ext_blq_inodos,fent);
      //GrabarByteMaps(&ext_bytemaps,fent);
      //GrabarSuperBloque(&ext_superblock,fent);
      if (grabardatos)
         //GrabarDatos(&memdatos,fent);
      grabardatos = 0;
      //Si el comando es salir se habrán escrito todos los metadatos
      //faltan los datos y cerrar
      if (strcmp(orden,"salir")==0)
      {
         //GrabarDatos(&memdatos,fent);
         fclose(fent);
         return 0;
      }
   }
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
   return 0;
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
   for(int i = 0; i < MAX_FICHEROS; i++){
      if((strcmp(directorio[i].dir_nfich, nombreantiguo) == 0)&&(strcmp(directorio[i].dir_nfich, nombrenuevo) != 0)){
         memcpy(directorio[i].dir_nfich, nombrenuevo, LEN_NFICH);
      }
   }
}

int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre)
{
    int inodo_a_imprimir = -1;

    // 1. Buscar la entrada del directorio correspondiente al nombre del fichero
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (strcmp(directorio[i].dir_nfich, nombre) == 0) {
            inodo_a_imprimir = directorio[i].dir_inodo;
            printf("Fichero '%s' encontrado en la entrada del directorio con inodo %d.\n", nombre, inodo_a_imprimir);
            break;
        }
    }

    // Si no se encuentra el fichero, devolver un error
    if (inodo_a_imprimir == -1) {
        printf("Error: fichero '%s' no encontrado.\n", nombre);
        return -1;  // Error: archivo no encontrado
    }

    // 2. Recuperar el inodo del fichero
    EXT_SIMPLE_INODE *inodo = &inodos->blq_inodos[inodo_a_imprimir];

    // Verificar si el fichero tiene contenido (tamaño mayor que 0)
    if (inodo->size_fichero == 0) {
        printf("El fichero '%s' está vacío.\n", nombre);
        return 0;  // El fichero está vacío
    }

    // 3. Mostrar el contenido del fichero bloque por bloque
    printf("Contenido de '%s' (tamaño: %d bytes):\n", nombre, inodo->size_fichero);
    int num_bloques = (inodo->size_fichero + SIZE_BLOQUE - 1) / SIZE_BLOQUE;  // Calcular cuántos bloques ocupa el fichero
    printf("El fichero ocupa %d bloques.\n", num_bloques);

    for (int i = 0; i < num_bloques; i++) {
        unsigned short int bloque = inodo->i_nbloque[i];

        // Verificar que el bloque no sea NULL_BLOQUE
        if (bloque == NULL_BLOQUE) {
            printf("Error: bloque inválido para el fichero '%s' en el índice %d.\n", nombre, i);
            return -1;  // Error: bloque inválido
        }

        // Leer el bloque de datos
        EXT_DATOS *bloque_dato = &memdatos[bloque];

        // Mostrar el contenido del bloque
        printf("Bloque %d (puntero a bloque: %d): ", i + 1, bloque);
        for (int j = 0; j < SIZE_BLOQUE; j++) {
            printf("%c", bloque_dato->dato[j] != 0 ? bloque_dato->dato[j] : '.'); // Usamos '.' para los bytes vacíos
        }
        printf("\n");
    }

    return 0;  // Éxito
}



int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, char *nombre, FILE *fich)

   {
    int inodo_a_eliminar = -1, salir = 0;

    // 1. Buscar la entrada del directorio correspondiente al nombre del fichero
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

    // 2. Liberar el inodo correspondiente
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

        // 3. Liberar los bloques de datos
        for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {
            if (inodo->i_nbloque[i] != NULL_BLOQUE) {
                // Marcar el bloque como libre en el bytemap de bloques
                ext_bytemaps->bmap_bloques[inodo->i_nbloque[i]] = 0;
            }
        }
    }

    // 4. Eliminar la entrada del directorio
    salir =0;
    for (int i = 0; i < MAX_FICHEROS && !salir; i++) {
        if (directorio[i].dir_inodo == inodo_a_eliminar) {
            // Poner el nombre vacío y el número de inodo a NULL_INODO
            memset(directorio[i].dir_nfich, 0, LEN_NFICH);
            directorio[i].dir_inodo = NULL_INODO;
            salir =1;
        }
    }

    // 5. Grabar los cambios en el fichero
    //Grabarinodosydirectorio(directorio, inodos, fich);
    //GrabarByteMaps(ext_bytemaps, fich);
    //GrabarSuperBloque(ext_superblock, fich);
    return 0;  // Éxito
}

int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino, FILE *fich)
{
    int inodo_origen = -1;
    int inodo_destino = -1;

    // 1. Buscar el fichero origen en el directorio
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (strcmp(directorio[i].dir_nfich, nombreorigen) == 0) {
            inodo_origen = directorio[i].dir_inodo;
            printf("Fichero origen '%s' encontrado en la entrada del directorio con inodo %d.\n", nombreorigen, inodo_origen);
            break;
        }
    }

    if (inodo_origen == -1) {
        printf("Error: fichero origen '%s' no encontrado.\n", nombreorigen);
        return -1;  // Error: fichero origen no encontrado
    }

    // 2. Verificar que el destino no existe en el directorio
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (strcmp(directorio[i].dir_nfich, nombredestino) == 0) {
            printf("Error: ya existe un fichero con el nombre '%s'.\n", nombredestino);
            return -1;  // Error: el fichero destino ya existe
        }
    }

    // 3. Buscar el primer inodo libre
    for (int i = 0; i < MAX_INODOS; i++) {
        if (inodos->blq_inodos[i].size_fichero == 0) {
            inodo_destino = i;
            break;
        }
    }

    if (inodo_destino == -1) {
        printf("Error: no hay inodos libres para crear el fichero '%s'.\n", nombredestino);
        return -1;  // Error: no hay inodos libres
    }

    // 4. Copiar el tamaño del fichero origen al inodo destino
    EXT_SIMPLE_INODE *inodo_origen_ptr = &inodos->blq_inodos[inodo_origen];
    EXT_SIMPLE_INODE *inodo_destino_ptr = &inodos->blq_inodos[inodo_destino];

    inodo_destino_ptr->size_fichero = inodo_origen_ptr->size_fichero;

    // 5. Copiar los bloques del fichero origen a bloques libres en el destino
    for (int i = 0; i < MAX_NUMS_BLOQUE_INODO; i++) {
        unsigned short int bloque_origen = inodo_origen_ptr->i_nbloque[i];
        if (bloque_origen != NULL_BLOQUE) {
            // Buscar primer bloque libre
            int bloque_destino = -1;
            for (int j = 0; j < MAX_BLOQUES_PARTICION; j++) {
                if (ext_bytemaps->bmap_bloques[j] == 0) {
                    bloque_destino = j;
                    break;
                }
            }

            if (bloque_destino == -1) {
                printf("Error: no hay bloques libres disponibles para el fichero '%s'.\n", nombredestino);
                return -1;  // Error: no hay bloques libres
            }

            // Asignar bloque al inodo destino
            inodo_destino_ptr->i_nbloque[i] = bloque_destino;

            // Marcar el bloque como ocupado en el bitmap de bloques
            ext_bytemaps->bmap_bloques[bloque_destino] = 1;

            // Copiar el contenido del bloque
            memcpy(&memdatos[bloque_destino], &memdatos[bloque_origen], SIZE_BLOQUE);
        }
    }

    // 6. Crear una nueva entrada en el directorio
    for (int i = 0; i < MAX_FICHEROS; i++) {
        if (directorio[i].dir_nfich[0] == '\0') {
            // Asignar el nombre y el inodo al nuevo fichero
            strcpy(directorio[i].dir_nfich, nombredestino);
            directorio[i].dir_inodo = inodo_destino;
            printf("Fichero '%s' copiado con éxito. Nuevo inodo: %d\n", nombredestino, inodo_destino);
            break;
        }
    }

    // 7. Guardar los cambios en disco (actualizar directorio, inodos, byte map de bloques)
    Grabarinodosydirectorio(directorio, inodos, fich);
    GrabarByteMaps(ext_bytemaps, fich);

    return 0;  // Éxito
}


void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich)
{
}

void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich)
{
}

void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich)
{
}

void GrabarDatos(EXT_DATOS *memdatos, FILE *fich)
{
}

