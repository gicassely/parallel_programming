#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <time.h>
#include <sys/times.h>
#include <sys/time.h>

#define INCLUDED 0
#define NOT_INCLUDED 1
#define max(a,b) (a>b?a:b)
#define MAIN_PROC 0
#define N_MAX 1000
#define VALOR_MAX 1000
#define PESO_MAX 1000
#define TAG 0x20

void outputs();
void inputs(int argc, char *argv[]);

int numproc, rank;
int capacidade_mochila, n, seed;
int *valor;
int *peso;
int table_cost[N_MAX][PESO_MAX];
int table_s[N_MAX][PESO_MAX];
int *map;
int colPerProc;


int main(int argc,char *argv[]) {
	int i, j, k;
	int valor_com_i, valor_sem_i;
	MPI_Status status;
	MPI_Request request;

	 /* Variaveis usadas para cronometrar o tempo */
  	struct timeval etstart, etstop;  /* Tempo decorrido usando gettimeofday() */
	struct timezone tzdummy;
	unsigned long long usecstart, usecstop;

/*É necessário inicializar o MPI, acessando o 
rank  e total de processos que estão sendo executados.*/
	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);//pegando nro do processos que esta executando aqui
	MPI_Comm_size(MPI_COMM_WORLD,&numproc);//pegando nro de processos

	valor = malloc(sizeof(int)*N_MAX);
	peso =(int*)malloc(sizeof(int)*N_MAX);
	map = (int*)malloc(sizeof(int)*N_MAX);
	
	inputs(argc, argv);	//vai para o carregamento dos dados
	MPI_Barrier(MPI_COMM_WORLD);

  	/* se é o proc 0 (ROOT_RANK) manda mensagem */
	if(rank==0) printf("\nIniciando Relógio.\n");
	gettimeofday(&etstart, &tzdummy);

	//Para cada item
	for(i=0;i<n;i++){
		// Cada rank calcula suas colunas  
		for(j=rank; j<capacidade_mochila; j+=numproc){

			//Encontre o melhor valor com o novo item
			if(j - peso[i]<0)//Se o peso do item for maior que o peso da coluna, o valor é 0
				valor_com_i=0;

			else if(i == 0 && j-peso[i] >= 0)//Se for o primeiro item e se o peso do item for menor que o peso da coluna, o valor é o valor do item
				valor_com_i=valor[i];

			else{
				MPI_Recv(&valor_com_i, 1, MPI_INT, map[j-peso[i]], i-1, MPI_COMM_WORLD, &status);// recebe a mensagem
				valor_com_i+=valor[i];//o valor do item mais o valor da célula [i-1] [j-peso [i]].
			}

			//Encontre o melhor valor sem o novo item
			valor_sem_i = (i==0) ? 0 : table_cost[i-1][j];//0 ou calculo da linha i-1

			//Calcular Tabela[i][j]s
			table_cost[i][j] = max(valor_com_i, valor_sem_i);// ou pega o valor do item + os calculos ou i-1
			table_s[i][j] = (table_cost[i][j]==valor_sem_i) ? NOT_INCLUDED : INCLUDED;

			//Enviar para todos os procs que precisariam o novo valor 
			for(k=j+1; k<capacidade_mochila; k++){
				if(k-j == peso[i+1]){
					MPI_Isend(&table_cost[i][j], 1, MPI_INT, map[k], i, MPI_COMM_WORLD, &request);
                 //int MPI_Isend(*buf, count, datatype, dest, tag, comm, *request)
				}
			}

		}
	}


	/* Fim de contagem do tempo */
	gettimeofday(&etstop, &tzdummy);
	if(rank==0) printf("Tempo encerrado.\n");
	usecstart = (unsigned long long)etstart.tv_sec * 1000000 + etstart.tv_usec;
	usecstop = (unsigned long long)etstop.tv_sec * 1000000 + etstop.tv_usec;

	MPI_Barrier(MPI_COMM_WORLD);// espera até ter o resultado completo
// então mostra os resultados

	outputs();

	/* Exibir resultados de tempo */
	if(rank==0) printf("\n time = %g ms.\n", (float)(usecstop - usecstart)/(float)1000);

	MPI_Finalize();
	return 0;
}


void inputs(int argc, char *argv[]){
	int i;

	if(argc != 4){
		if(rank == MAIN_PROC)
			printf("Erro:  mpirun -np processos ./executavel <capacidade máxima> <número de itens> <seed>\n");
		MPI_Finalize();
		exit(0);
	}

	capacidade_mochila = atoi(argv[1])+1;
	n  = atoi(argv[2]);
	seed = atoi(argv[3]);
	
	if(n>N_MAX){
		if(rank == MAIN_PROC)
			printf("Erro: Maximo de intens eh: %d\n", N_MAX);
		MPI_Finalize();
		exit(0);
	}
	if(capacidade_mochila>PESO_MAX+1){
		if(rank == MAIN_PROC)
			printf("Erro: A capacdade maxima eh: %d\n", PESO_MAX);
		MPI_Finalize();
		exit(0);
	}

	srandom(seed);

	for(i=0;i<n;i++){
		valor[i]=random()%VALOR_MAX;
		peso[i]=random()%(capacidade_mochila-1)+1;
		//if(rank == MAIN_PROC)
			//printf("\tItem %d: valor = %d / peso = %d\n", i, valor[i], peso[i]);
	}


	for (i = 0; i < capacidade_mochila; i++){
		map[i] = i%numproc;
	}

}

void outputs(){
	int i, j;
	MPI_Status status;
	MPI_Request request;

	int table_svg[N_MAX][PESO_MAX];

	// Assimila resultados e imprima-os
	if(rank != map[capacidade_mochila-1]){
		for(j=rank; j<capacidade_mochila-1; j+=numproc){
			MPI_Isend(table_s, N_MAX*PESO_MAX, MPI_INT, map[capacidade_mochila-1], TAG, MPI_COMM_WORLD, &request);
		}
	}else{

		memcpy(table_svg, table_s, N_MAX*PESO_MAX);

		printf("\n\nResultados:\n");
		printf("\tPeso máximo: %d / Numero de itens: %d\n", capacidade_mochila-1, n);
		printf("\tMelhor valor: %d \n", table_cost[n-1][capacidade_mochila-1]);
		printf("\t Itens utilizados: ");
		
		j=capacidade_mochila-1;

		for(i=n-1; j>0 && i>=0; i--){
			if(table_s[i][j] == INCLUDED){
				printf("%d, ",i);

				j-=peso[i];
				
				if(j > 0 ){
					if (map[j]!=rank) {// Se o rank for diferente ele recebe o resultado 
						MPI_Recv(table_s, N_MAX*PESO_MAX, MPI_INT, map[j], TAG, MPI_COMM_WORLD, &status);
					} else {
						memcpy(table_s, table_svg, N_MAX*PESO_MAX);
					}

				}
			}
		}
		printf("\n\n");
		
	}
}
