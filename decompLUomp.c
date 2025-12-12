#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main(int argc, char*argv[])
{
	int n = atoi(argv[1]); //tamanho da matriz
	int t = atoi(argv[2]); //numero de threads
	int aux=(n/2)*n;
	double U[n][n], C[aux];
	int i, j, k, cont =0;
	int l=0;
	double c,  soma=0.0;

	omp_set_num_threads(t);

	for(i=0; i<n;i++) //criando a matriz
	{
		for(j=0;j<n;j++)
		{
			U[i][j]=rand()%100;
		}
	}
	printf("\nMatriz A\n");
	for(i=0;i<n;i++)
	{
		for(j=0;j<n;j++)
		{
			printf("%f  ", U[i][j]);
		}
		printf("\n");
	}
	double start, end;
	start = omp_get_wtime();
	for(j=0; j<n; j++)
	{
		for(i=0; i<n; i++)
		{
			if(i>j)
			{
				c=-(U[i][j]/U[j][j]);
				#pragma omp parallel
				{
					#pragma omp for
					for(k=0; k<n; k++)
			 		{
						U[i][k]=c*U[j][k]+U[i][k];
					}
				}
				C[l]=-c;
				l++;
			}
		}
	}

	printf(" \nMatriz U\n ");
	for(i=0;i<n;i++)
	{
		for(j=0;j<n;j++)
		{
			printf("%0.1f  ", U[i][j]);
		}
		printf("\n ");
	}
	/*printf(" \n Vetor C\n ");
	for(j=0;j<aux;j++)
	{
		printf("%0.1f  ", C[j]);
	}*/
	#pragma omp parallel for private (j)
		for(i=0; i<=n;i++)//matriz L
		{
			for(j=0;j<n;j++)
			{
				if(i==j)
				{
					U[i][j]=1.0;
				}
				else
				{
					if(j>i)
						U[i][j]=0.0;
				}
			}
		}
	k=aux/t;
	int f;
	#pragma omp parallel private(f, j)//valores de f e j privados
	{
		int id=omp_get_thread_num();
		f=id*k; //off set do vetor de pivos
		#pragma omp for
		for(i=1;i<n;i++)
		{
			for(j=0;j<n;j++)
			{
				if(i>j)
				{
					U[i][j]=C[f];
					f++; 
				}
			}
		}
	}
	end = omp_get_wtime();
	double tempo = end-start;
	printf("\n Matriz L \n ");
	for(i=0;i<n;i++)
	{
		for(j=0;j<n;j++)
		{
			printf("%0.1f  ", U[i][j]);
		}
		printf("\n ");
	}
	printf("\ntempo de execução: %f\n", tempo);
}
