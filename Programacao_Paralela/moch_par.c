#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <omp.h>

// Função que retorna o máximo de dois inteiros
int max(int a, int b) { return (a > b)? a : b; }

// Retorna o valorue máximo que pode ser colocado em uma mochila de capacidade W
static int knapSack(long int W, long int N, int peso[], int valor[])
{
	int *K = (int*) malloc((W+1)*sizeof(int));
	int *Kp = (int*) malloc((W+1)*sizeof(int));
	long int i, j;

	for (i = 0; i <= N; i++)
	{
		#pragma omp parallel for // <--------------------- Metodo de parelelizacao utilizado  
		for (j = 0; j <= W; j++)
		{
			if (i==0 || j==0)
				K[j] = 0;
			else if (peso[i-1] <= j)
				K[j] = max(valor[i-1] + Kp[j-peso[i-1]], Kp[j]);
			else
				K[j] = Kp[j];
		}
		int *tmp = Kp;
		Kp = K;
		K = tmp;
	}

	int resultado = K[W];

	free(K);
	free(Kp);

	return resultado;
}

int main(int argc, char **argv){

	FILE *arquivo;

	int *valor, *peso;   
	long int Nitems; // numero de itens
	long int Width;  // Máx. Carga 
	long int cont;    
	double tempo1=0,tempo2=0,tempo3=0; 
	struct timeval tim;

	if (argc < 2) {
		printf("Usage: ./%s <arquivo> <numerero de processos>\n", argv[0]);
		return 1;
	}

	if (argc == 3) {
		omp_set_num_threads(atoi(argv[2]));
	}

	// Captura o primeiro tempo da execução 
	gettimeofday(&tim, NULL);
	tempo1 = tim.tv_sec+(tim.tv_usec/1000000.0);

	if (!(arquivo=fopen(argv[1],"r"))) {
		printf("Erro ao abrir o arquivo: %s\n",argv[1]);
		return 1;
	}

	//Lendo o numero de itens e capacidade maxima da mochila
	fscanf(arquivo,"%ld %ld\n",&Nitems, &Width);

	valor = (int *)malloc(Nitems*sizeof(int)); 
	peso = (int *)malloc(Nitems*sizeof(int)); 

	//Leitura do arquivo do valor e peso de cada item
	for (cont=0;cont<Nitems;cont++){
		fscanf(arquivo,"%d,%d\n",&valor[cont],&peso[cont]);
	}

	gettimeofday(&tim, NULL);
	tempo2 = (tim.tv_sec+(tim.tv_usec/1000000.0));
	printf("%ld:%ld:%d", Width, Nitems, knapSack(Width,Nitems, peso, valor));
	gettimeofday(&tim, NULL);
	tempo3 = (tim.tv_sec+(tim.tv_usec/1000000.0));
	printf(":%.6lf:%.6lf\n", tempo3-tempo2,tempo3-tempo1);

	free(valor);
	free(peso);

	fclose(arquivo);

	return 0;
}

