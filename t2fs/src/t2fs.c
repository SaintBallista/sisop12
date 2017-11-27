#include "../include/t2fs.h"
#include "../include/apidisk.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BYTES_PER_SECTOR 256

#define MAX_LAA 10

typedef struct fileHandler{
	int fileHandle;	//handle do arquivo
	int posFile;		//current position pointer do arquivo
	struct t2fs_record *fileRecord;		//pointer para o record do arquivo
} Handler;

struct t2fs_superbloco *superbloco;
int initialized = 0, handlerCount = 0;
char *current_path= "/";
struct t2fs_record *rootDir;
struct t2fs_record *currentDir;

Handler* lista_arq_abertos[MAX_LAA] = { NULL,NULL,NULL,NULL,NULL,
												NULL,NULL,NULL,NULL,NULL };


#define INIT {initialize();}


/*****************************************************************************
*******************FUNÇÃOS AUXILIARES******************************************
********************************************************nigger*****************
*/

WORD wordConvert(int *pos, BYTE *buffer){
	int i;
	WORD bird, auxbuffer[2] = {0};
	for (i = 0; i < 2; i++){
		auxbuffer[i] = buffer[i+(*pos)];
	}
	bird = (unsigned short int) (auxbuffer[0] | (auxbuffer[1] << 8));
	(*pos) += 2;
	return bird;
}

DWORD dWordConvert(int *pos, BYTE *buffer){
	int i;
	DWORD birdbird, auxbuffer[4] = {0};
	for (i = 0; i < 4; i++){
		//theword = buffer[i+(*pos)];  //whatever the fuck is going on here
		auxbuffer[i] = buffer[i+(*pos)];
	}
	birdbird = (unsigned int) (auxbuffer[0] | (auxbuffer[1] << 8) | (auxbuffer[2] << 16) | (auxbuffer[3] << 24));
	(*pos) += 4;
	return birdbird;
}

void initialize(){
	int i, *curr_pos, sector_aux, cluster_pos;
	BYTE *buffer = (char*) malloc(SECTOR_SIZE);

	if (!initialized){
		read_sector(0, buffer);
		for (i = 0; i < 4; i++){
			superbloco->id[i] = buffer[3-i];
		}
		(*curr_pos) = i + 1;
		superbloco->version = wordConvert(curr_pos, buffer);
		superbloco->SuperBlockSize = wordConvert(curr_pos, buffer);
		superbloco->DiskSize = dWordConvert(curr_pos, buffer);
		superbloco->NofSectors  = dWordConvert(curr_pos, buffer);
		superbloco->SectorsPerCluster = dWordConvert(curr_pos, buffer);
		superbloco->pFATSectorStart = dWordConvert(curr_pos, buffer);
		superbloco->RootDirCluster = dWordConvert(curr_pos, buffer);
		superbloco->DataSectorStart = dWordConvert(curr_pos, buffer);
		sector_aux = superbloco->RootDirCluster / 4;
		read_sector(sector_aux, buffer);
		cluster_pos = 64 * (superbloco->RootDirCluster % 4);
		rootDir->TypeVal = buffer[cluster_pos];
		for(i = cluster_pos; i < (MAX_FILE_NAME_SIZE + cluster_pos); i++){
			rootDir->name[i] = buffer[MAX_FILE_NAME_SIZE + cluster_pos - i];
		}
		(*curr_pos) = MAX_FILE_NAME_SIZE + cluster_pos;
		rootDir->bytesFileSize = dWordConvert(curr_pos, buffer);
		rootDir->firstCluster = dWordConvert(curr_pos, buffer);
		initialized = 1;
	}
}


int insereListaArqAbertos(struct t2fs_record* novo_record){
	int i=0;

	while((i<MAX_LAA) && (lista_arq_abertos[i] != NULL)){
		i++;
	}

	if (lista_arq_abertos[i] == NULL){
		Handler *handler = malloc(sizeof(Handler));

		handler->fileHandle = i;
		handler->posFile;
		handler->fileRecord = novo_record;

		lista_arq_abertos[i] = handler;
		//(*lista_arq_abertos[i]) = (t2fs_record) malloc(sizeof(t2fs_record));

		//(*lista_arq_abertos[i])->TypeVal = novo_record->TypeVal;
		//strcpy((*lista_arq_abertos[i])->name,novo_record->name )
		//(*lista_arq_abertos[i])->bytesFileSize = novo_record->bytesFileSize;
		//(*lista_arq_abertos[i])->firstCluster = novo_record->firstCluster;


		return i; //execução terminou com sucesso, devolve o handler
	}

	return -1; //execução acabou com erros

}



DWORD procuraClusterVazio(){
	int i = superbloco->pFATSectorStart, index = 0, cluster_index=-1;
	int flagAchou =0;
	char *buffer = malloc(SECTOR_SIZE);
	DWORD cluster;
	while((i<superbloco->DataSectorStart) && (read_sector(i, buffer)==0) && (flagAchou!=1)){
		int j;
		i++;
		index = 0;
		for(j=0; j<64; j++){
			cluster = dWordConvert(&index, buffer);
			cluster_index++;
			printf("%u\n", cluster);
			if (cluster == 0x00000000){
				flagAchou=1;
				break;
			}
		}
	}
	if (flagAchou==1)
		return cluster_index;

	return -1;
}

int getFileRecord(struct t2fs_record* directory, char* filename, struct t2fs_record* file){
	struct t2fs_record record[4];
	int found, cluster, cluster_aux, i, j;
	unsigned int sector, data_sector;
	BYTE *buffer, *buffer_aux, *buffer_record;
	DWORD next_cluster;

	// Read starter cluster from FAT
	sector = 1 + (directory->firstCluster / 64);
	cluster = directory->firstCluster % 64;
	while(!found){
		read_sector(sector, buffer);
		cluster_aux = cluster;
		for (i = 0; i < 4; i++){
			next_cluster = dWordConvert(&cluster_aux, buffer);
		}
		// Read the cluster (N sectors per cluster, so N*4 records per cluster)
		data_sector = superbloco->DataSectorStart + (cluster * (superbloco->SectorsPerCluster));
		for (i = 0; i < superbloco->SectorsPerCluster; i++){
			read_sector(data_sector + i, buffer);
			for (j = 0; j < 4; j++){
				//record[i] = (struct t2fs_record*) buffer[64*i];
				/*if(strcmp(record[i]->name, filename)){
					found = 1;
					file = record[i];
				}*/
			}
		}
		// Set up the next FAT SECTOR TO CHECK!
		sector = 1 + (next_cluster/64);
		cluster = next_cluster % 64;
	}
	return -1;
}


// FUN��ES PRINCIPAIS DA BILBIOTECA


int identify2 (char *name, int size){ INIT;
	int i;
	char *ident = "Fernando Garcia Bock 242255\nLeonardo Wellausen 261571\nJoao Batista Manique Henz 242251\n\0";

	if (size >= sizeof(ident)){
		strcpy(name, ident);
		return 0;
	}
	else{
		strncpy(name, ident, size);
		return 0;
	}

	return -1;
	// PLACEHOLDER DE FUN��O TBQH
	/*-----------------------------------------------------------------------------
	Fun��o: Usada para identificar os desenvolvedores do T2FS.
		Essa fun��o copia um string de identifica��o para o ponteiro indicado por "name".
		Essa c�pia n�o pode exceder o tamanho do buffer, informado pelo par�metro "size".
		O string deve ser formado apenas por caracteres ASCII (Valores entre 0x20 e 0x7A) e terminado por �\0�.
		O string deve conter o nome e n�mero do cart�o dos participantes do grupo.

	Entra:	name -> buffer onde colocar o string de identifica��o.
		size -> tamanho do buffer "name" (n�mero m�ximo de bytes a serem copiados).

	Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna "0" (zero).
		Em caso de erro, ser� retornado um valor diferente de zero.
	-----------------------------------------------------------------------------*/
}



FILE2 create2 (char *filename){ INIT;
	if (handlerCount>=MAX_LAA){
		return -1;  //não há mais espaço para abrir arquivos
	}
	handlerCount++;

	struct t2fs_record* novo_record = malloc(sizeof(struct t2fs_record));

	novo_record->TypeVal = 0x01;//tipo arquivo simples
	strcpy(novo_record->name,filename);
	novo_record->bytesFileSize = 0;
	novo_record->firstCluster = procuraClusterVazio() ;

	if(novo_record->firstCluster != -1){
		int handle = insereListaArqAbertos(novo_record);
		if(handle)
			return handle;
	}

	//TODO arrumar estrutura de diretório

	return -1;
	/*-----------------------------------------------------------------------------
	Fun��o: Criar um novo arquivo.
		O nome desse novo arquivo � aquele informado pelo par�metro "filename".
		O contador de posi��o do arquivo (current pointer) deve ser colocado na posi��o zero.
		Caso j� exista um arquivo ou diret�rio com o mesmo nome, a fun��o dever� retornar um erro de cria��o.
		A fun��o deve retornar o identificador (handle) do arquivo.
		Esse handle ser� usado em chamadas posteriores do sistema de arquivo para fins de manipula��o do arquivo criado.

	Entra:	filename -> path absoluto para o arquivo a ser criado. Todo o "path" deve existir.

	Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna o handle do arquivo (n�mero positivo).
		Em caso de erro, deve ser retornado um valor negativo.
	-----------------------------------------------------------------------------*/
}



int delete2 (char *filename){ INIT;
	return -1;
	/*-----------------------------------------------------------------------------
	Fun��o:	Apagar um arquivo do disco.
		O nome do arquivo a ser apagado � aquele informado pelo par�metro "filename".

	Entra:	filename -> nome do arquivo a ser apagado.

	Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna "0" (zero).
		Em caso de erro, ser� retornado um valor diferente de zero.
	-----------------------------------------------------------------------------*/
}



FILE2 open2 (char *filename){ INIT;
	struct t2fs_record novo_record;

	// acessando arquivo no diretório pai
	if (filename[0] == '.' && filename[1] == '.'){
		struct t2fs_record novo_dir;
		//getDirRecord();

	}
	// usando caminho absoluto para outro diretório
	else if (filename [0] == '/'){

	}
	// acessando arquivo no diretório atual
	else{
		//getFileRecord(&currentDir, filename);
	}


	return -1;
	/*-----------------------------------------------------------------------------
	Fun��o:	Abre um arquivo existente no disco.
		O nome desse novo arquivo � aquele informado pelo par�metro "filename".
		Ao abrir um arquivo, o contador de posi��o do arquivo (current pointer) deve ser colocado na posi��o zero.
		A fun��o deve retornar o identificador (handle) do arquivo.
		Esse handle ser� usado em chamadas posteriores do sistema de arquivo para fins de manipula��o do arquivo criado.
		Todos os arquivos abertos por esta chamada s�o abertos em leitura e em escrita.
		O ponto em que a leitura, ou escrita, ser� realizada � fornecido pelo valor current_pointer (ver fun��o seek2).

	Entra:	filename -> nome do arquivo a ser apagado.

	Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna o handle do arquivo (n�mero positivo)
		Em caso de erro, deve ser retornado um valor negativo
	-----------------------------------------------------------------------------*/
}



int close2 (FILE2 handle){ INIT;
	return -1;
	/*-----------------------------------------------------------------------------
	Fun��o:	Fecha o arquivo identificado pelo par�metro "handle".

	Entra:	handle -> identificador do arquivo a ser fechado

	Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna "0" (zero).
		Em caso de erro, ser� retornado um valor diferente de zero.
	-----------------------------------------------------------------------------*/
}



int read2 (FILE2 handle, char *buffer, int size){ INIT;
	return -1;
	/*-----------------------------------------------------------------------------
	Fun��o:	Realiza a leitura de "size" bytes do arquivo identificado por "handle".
		Os bytes lidos s�o colocados na �rea apontada por "buffer".
		Ap�s a leitura, o contador de posi��o (current pointer) deve ser ajustado para o byte seguinte ao �ltimo lido.

	Entra:	handle -> identificador do arquivo a ser lido
		buffer -> buffer onde colocar os bytes lidos do arquivo
		size -> n�mero de bytes a serem lidos

	Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna o n�mero de bytes lidos.
		Se o valor retornado for menor do que "size", ent�o o contador de posi��o atingiu o final do arquivo.
		Em caso de erro, ser� retornado um valor negativo.
	-----------------------------------------------------------------------------*/
}



int write2 (FILE2 handle, char *buffer, int size){ INIT;
	return -1;
	/*-----------------------------------------------------------------------------
	Fun��o:	Realiza a escrita de "size" bytes no arquivo identificado por "handle".
		Os bytes a serem escritos est�o na �rea apontada por "buffer".
		Ap�s a escrita, o contador de posi��o (current pointer) deve ser ajustado para o byte seguinte ao �ltimo escrito.

	Entra:	handle -> identificador do arquivo a ser escrito
		buffer -> buffer de onde pegar os bytes a serem escritos no arquivo
		size -> n�mero de bytes a serem escritos

	Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna o n�mero de bytes efetivamente escritos.
		Em caso de erro, ser� retornado um valor negativo.
	-----------------------------------------------------------------------------*/
}



int truncate2 (FILE2 handle){ INIT;
	return -1;
	/*-----------------------------------------------------------------------------
	Fun��o:	Fun��o usada para truncar um arquivo.
		Remove do arquivo todos os bytes a partir da posi��o atual do contador de posi��o (CP)
		Todos os bytes a partir da posi��o CP (inclusive) ser�o removidos do arquivo.
		Ap�s a opera��o, o arquivo dever� contar com CP bytes e o ponteiro estar� no final do arquivo

	Entra:	handle -> identificador do arquivo a ser truncado

	Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna "0" (zero).
		Em caso de erro, ser� retornado um valor diferente de zero.
	-----------------------------------------------------------------------------*/
}



int seek2 (FILE2 handle, unsigned int offset){ INIT;
	return -1;
	/*-----------------------------------------------------------------------------
	Fun��o:	Reposiciona o contador de posi��es (current pointer) do arquivo identificado por "handle".
		A nova posi��o � determinada pelo par�metro "offset".
		O par�metro "offset" corresponde ao deslocamento, em bytes, contados a partir do in�cio do arquivo.
		Se o valor de "offset" for "-1", o current_pointer dever� ser posicionado no byte seguinte ao final do arquivo,
			Isso � �til para permitir que novos dados sejam adicionados no final de um arquivo j� existente.

	Entra:	handle -> identificador do arquivo a ser escrito
		offset -> deslocamento, em bytes, onde posicionar o "current pointer".

	Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna "0" (zero).
		Em caso de erro, ser� retornado um valor diferente de zero.
	-----------------------------------------------------------------------------*/
}



int mkdir2 (char *pathname){ INIT;
	return -1;
	/*-----------------------------------------------------------------------------
	Fun��o:	Criar um novo diret�rio.
		O caminho desse novo diret�rio � aquele informado pelo par�metro "pathname".
			O caminho pode ser ser absoluto ou relativo.
		S�o considerados erros de cria��o quaisquer situa��es em que o diret�rio n�o possa ser criado.
			Isso inclui a exist�ncia de um arquivo ou diret�rio com o mesmo "pathname".

	Entra:	pathname -> caminho do diret�rio a ser criado

	Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna "0" (zero).
		Em caso de erro, ser� retornado um valor diferente de zero.
	-----------------------------------------------------------------------------*/
}



int rmdir2 (char *pathname){ INIT;
	return -1;
	/*-----------------------------------------------------------------------------
	Fun��o:	Apagar um subdiret�rio do disco.
		O caminho do diret�rio a ser apagado � aquele informado pelo par�metro "pathname".
		S�o considerados erros quaisquer situa��es que impe�am a opera��o.
			Isso inclui:
				(a) o diret�rio a ser removido n�o est� vazio;
				(b) "pathname" n�o existente;
				(c) algum dos componentes do "pathname" n�o existe (caminho inv�lido);
				(d) o "pathname" indicado n�o � um arquivo;

	Entra:	pathname -> caminho do diret�rio a ser criado

	Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna "0" (zero).
		Em caso de erro, ser� retornado um valor diferente de zero.
	-----------------------------------------------------------------------------*/
}



int chdir2 (char *pathname){ INIT;
	return -1;
	/*-----------------------------------------------------------------------------
	Fun��o:	Altera o current path
		O novo caminho do diret�rio a ser usado como current path � aquele informado pelo par�metro "pathname".
		S�o considerados erros quaisquer situa��es que impe�am a opera��o.
			Isso inclui:
				(a) "pathname" n�o existente;
				(b) algum dos componentes do "pathname" n�o existe (caminho inv�lido);
				(c) o "pathname" indicado n�o � um diret�rio;

	Entra:	pathname -> caminho do diret�rio a ser criado

	Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna "0" (zero).
		Em caso de erro, ser� retornado um valor diferente de zero.
	-----------------------------------------------------------------------------*/
}



int getcwd2 (char *pathname, int size){ INIT;
	return -1;
	/*-----------------------------------------------------------------------------
	Fun��o:	Fun��o que informa o diret�rio atual de trabalho.
		O T2FS deve copiar o pathname do diret�rio de trabalho, incluindo o �\0� do final do string, para o buffer indicado por "pathname".
		Essa c�pia n�o pode exceder o tamanho do buffer, informado pelo par�metro "size".

	Entra:	pathname -> ponteiro para buffer onde copiar o pathname
		size -> tamanho do buffer "pathname" (n�mero m�ximo de bytes a serem copiados).

	Sa�da:
		Se a opera��o foi realizada com sucesso, a fun��o retorna "0" (zero).
		Caso ocorra algum erro, a fun��o retorna um valor diferente de zero.
		S�o considerados erros quaisquer situa��es que impe�am a c�pia do pathname do diret�rio de trabalho para "pathname",
		incluindo espa�o insuficiente, conforme informado por "size".
	-----------------------------------------------------------------------------*/
}



DIR2 opendir2 (char *pathname){ INIT;
	return -1;
	/*-----------------------------------------------------------------------------
	Fun��o:	Abre um diret�rio existente no disco.
		O caminho desse diret�rio � aquele informado pelo par�metro "pathname".
		Se a opera��o foi realizada com sucesso, a fun��o:
			(a) deve retornar o identificador (handle) do diret�rio
			(b) deve posicionar o ponteiro de entradas (current entry) na primeira posi��o v�lida do diret�rio "pathname".
		O handle retornado ser� usado em chamadas posteriores do sistema de arquivo para fins de manipula��o do diret�rio.

	Entra:	pathname -> caminho do diret�rio a ser aberto

	Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna o identificador do diret�rio (handle).
		Em caso de erro, ser� retornado um valor negativo.
	-----------------------------------------------------------------------------*/
}




int readdir2 (DIR2 handle, DIRENT2 *dentry){ INIT;
	return -1;
	/*-----------------------------------------------------------------------------
	Fun��o:	Realiza a leitura das entradas do diret�rio identificado por "handle".
		A cada chamada da fun��o � lida a entrada seguinte do diret�rio representado pelo identificador "handle".
		Algumas das informa��es dessas entradas devem ser colocadas no par�metro "dentry".
		Ap�s realizada a leitura de uma entrada, o ponteiro de entradas (current entry) � ajustado para a pr�xima entrada v�lida
		S�o considerados erros:
			(a) t�rmino das entradas v�lidas do diret�rio identificado por "handle".
			(b) qualquer situa��o que impe�a a realiza��o da opera��o

	Entra:	handle -> identificador do diret�rio cujas entradas deseja-se ler.
		dentry -> estrutura de dados onde a fun��o coloca as informa��es da entrada lida.

	Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna "0" (zero).
		Em caso de erro, ser� retornado um valor negativo
			Se o diret�rio chegou ao final, retorna "-END_OF_DIR" (-1)
			Outros erros, ser� retornado um outro valor negativo


		#define	END_OF_DIR	1
	-----------------------------------------------------------------------------*/
}



int closedir2 (DIR2 handle){ INIT;
	return -1;
	/*-----------------------------------------------------------------------------
	Fun��o:	Fecha o diret�rio identificado pelo par�metro "handle".

	Entra:	handle -> identificador do diret�rio que se deseja fechar (encerrar a opera��o).

	Sa�da:	Se a opera��o foi realizada com sucesso, a fun��o retorna "0" (zero).
		Em caso de erro, ser� retornado um valor diferente de zero.
	-----------------------------------------------------------------------------*/
}

int main(int argc, char const *argv[]) {
	/* temp main for testin */
	//INIT;
	superbloco = (struct t2fs_superbloco *) malloc(256);
	read_sector(0,(char *) superbloco);
	char *s = malloc(256);
	char *name = malloc(128);
	int i;

	printf("Id:");
	for (i = 0; i < 4; i++){
		printf("%c",(unsigned char) superbloco->id[i]);
	}
	printf("\n");
	printf("Version: %x\n",(unsigned short int) superbloco->version);
	printf("Size of Superblock: %x\n",(unsigned short int) superbloco->SuperBlockSize);
	printf("Disk Size: %u\n",(unsigned short int) superbloco->DiskSize);
	printf("Number of Sectors: %u\n",(unsigned short int) superbloco->NofSectors);
	printf("Sectors Per Cluster: %u\n",(unsigned short int) superbloco->SectorsPerCluster);
	printf("FAT Start Sector: %u\n",(unsigned short int) superbloco->pFATSectorStart);
	printf("Root Directory Start Cluster: %u\n",(unsigned short int) superbloco->RootDirCluster);
	printf("Data Start Sector: %u\n",(unsigned short int) superbloco->DataSectorStart);
	//getchar();
	//identify2(name, 128);
	/* TESTE DE IMPRESSÃO DO NOME
	i = 0;
	while(name[i] != '\0'){
		printf("%c", name[i]);
		i++;
	}
	*/

	printf("%u\n",procuraClusterVazio());

	return 0;
}

//END OF FILE
