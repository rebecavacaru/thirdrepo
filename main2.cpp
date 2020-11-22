#define _CRT_SECURE_NO_WARNINGS
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
	double time = 0, time2 = 0, time3 = 0;
	int world_size, world_rank, r1, c1, r2, c2;

	FILE* m1 = fopen("C:/Users/rebeca.vacaru/Documents/matrix/qqq.txt", "r");
	fscanf(m1, "%d %d", &r1, &c1);
	FILE* m2 = fopen("C:/Users/rebeca.vacaru/Documents/matrix/m12.txt", "r");
	fscanf(m2, "%d %d", &r2, &c2);
	FILE* m3 = fopen("C:/Users/rebeca.vacaru/Documents/matrix/new.txt", "w");
	fprintf(m3, "%d %d\n", r1, c2);

	MPI_Init(NULL, NULL);
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

	if (c1 != r2 || world_size > r1) {
		fprintf(stderr, "Not good..\n");
		exit(-1);
	}

	int maxScat = c1 * (int)(r1 / world_size);
	int* A = NULL;
	int* Aq = (int*)malloc(maxScat * sizeof(int));
	int* B = (int*)malloc(r2 * c2 * sizeof(int));
	int* C = NULL;
	int* Cq = (int*)(malloc(maxScat / c1 * c2 * sizeof(int)));

	if (world_rank == 0) {
		A = (int*)malloc(r1 * c1 * sizeof(int));
		for (int i = 0; i < r1 * c1; i++) {
			fscanf(m1, "%d", A + i);
		}
		for (int i = 0; i < r2 * c2; i++) {
			fscanf(m2, "%d", B + i);
		}
		C = (int*)malloc(r1 * c2 * sizeof(int));
	}

	time2 = MPI_Wtime();
	MPI_Scatter(A, maxScat, MPI_INT, Aq, maxScat, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(B, r2 * c2, MPI_INT, 0, MPI_COMM_WORLD);
	time = MPI_Wtime();

	for (int i = 0; i < maxScat / c1; i++) {
		for (int q = 0; q < c2; q++) {
			int durere = 0;
			for (int j = 0; j < c1; j++) {
				durere += *(Aq + i * c1 + j) * *(B + j * c2 + q);
			}
			*(Cq + i * c2 + q) = durere;
		}
	}

	time = MPI_Wtime() - time;
	MPI_Gather(Cq, maxScat / c1 * c2, MPI_INT, C, maxScat / c1 * c2, MPI_INT, 0, MPI_COMM_WORLD);
	time2 = MPI_Wtime() - time2;

	if (world_rank == 0) {
		time3 = MPI_Wtime();
		for (int i = maxScat * world_size / c1; i < r1; i++) {
			for (int q = 0; q < c2; q++) {
				int durere = 0;
				for (int j = 0; j < c1; j++) {
					durere += *(A + i * c1 + j) * *(B + j * c2 + q);
				}
				*(C + i * c2 + q) = durere;
			}
		}
		time += MPI_Wtime() - time3;
		printf("It took %f to compute the product and %f to comunicate", time, time2 - time);
		for (int i = 0; i < r1 * c2; i++) {
			if (i % c2 == 0 && i != 0)
				fprintf(m3, "\n");
			fprintf(m3, "%d ", C[i]);
		}
	}
	MPI_Finalize();
}
