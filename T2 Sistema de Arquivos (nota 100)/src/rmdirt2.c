#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "t2fs.h"
#include "funcExtras.h"

int main(int argc,char *argv[])
{
	if(argc < 2)
	{
		printf("ERRO - Falta o path do diretorio\n");
		return -1;
	}
	if(argc > 2)
	{
		printf("ERRO - Muitos parametros\n");
		return -1;
	}
	char *dirT2FS = malloc(strlen(argv[1])+1);
	char *temp = malloc(strlen(argv[1])+1);

	int i, UltimaBarra, foundDir, deletado;

	strcpy(dirT2FS, argv[1]);

	/**
		Verifica se diretório existe
	*/
	strcpy(temp,dirT2FS);
	//printf("temp: %s\n", temp);
	for(i=strlen(dirT2FS); i>0;i--)
	{
		if(*(temp+i) == '/')
		{
			UltimaBarra = i;
			//printf("temp Atual: %s\n",temp+i);
			break;
		}
	}
	*(temp+i) = '\0';
	//printf("temp: %s\n", temp);
	foundDir = findDiretorio(temp);
	//printf("flag : %d\n", foundDir);
	if(foundDir == -1)
	{
		//Não achou o arquivo especificado
		printf("ERRO - Não achou algum diretorio do path\n");
		return -1;
	}
	
	/**
		deleta arquivo
	*/
	
	deletado = RemoveDiretorio(dirT2FS);
//	deletado = t2fs_delete(dirT2FS);
	if(deletado == -1)
	{
		printf("ERRO - Não conseguiu deletar o diretório do t2fs\n");
		return -1;
	}

	printf("Deletado\n");
	free(temp);
	temp = NULL;	
	free(dirT2FS);
	dirT2FS = NULL;

	return 0;
}
