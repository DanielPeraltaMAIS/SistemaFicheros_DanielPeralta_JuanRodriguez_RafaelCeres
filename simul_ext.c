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
int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, 
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

int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombre)
{
    return 0;
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



int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombreantiguo, char *nombrenuevo)
{
    return 0;
}

int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre)
{
    return 0;
}

int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, char *nombre, FILE *fich)
{
    return 0;
}

int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino, FILE *fich)
{
    return 0;
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
