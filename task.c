#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include <time.h>
#include <stdio.h>      /* printf */
#include <time.h>       /* clock_t, clock, CLOCKS_PER_SEC */
#include <math.h> 
#define SIZE_VECTOR 10
#define ROW_MATRIX1 1500
#define ROW_MATRIX2 1500
#define COLUMN_MATRIX1 1500
#define COLUMN_MATRIX2 1500
#define num_thread 16

struct vectorClass
{
	int column;
	int *__restrict vector;
};
struct matrixClass
{
	int column;
	int row;
	int *__restrict matrix;

};
struct matrixClass result;
struct arg_struct {
	struct matrixClass m1;
	struct matrixClass m2;
	int slice;
};
struct arg_struct_vm {
	struct vectorClass v1;
	struct matrixClass m2;
	int slice;
};

void vectorClass_allocate(struct vectorClass, int, int);
void vectorClass_show(struct vectorClass);
void vectorClass_init(struct vectorClass);

void matrixClass_allocate(struct matrixClass, int, int, int);
void matrixClass_show(struct matrixClass);
void matrixClass_init(struct matrixClass);
void *mult_matrix(void *);
void * mult_vector(void *);

void vectorClass_allocate(struct vectorClass v, int position, int value)
{
	v.vector[position] = value;
}
void vectorClass_show(struct vectorClass v)
{
	for (int i = 0; i < v.column; i++)
		printf("\t\t|\t%d\t|\n", v.vector[i]);
}
void vectorClass_init(struct vectorClass v)
{
	for (int position = 0; position < v.column; position++)
	{
		v.vector[position] = 0;
	}

}
void matrixClass_allocate(struct matrixClass m, int row, int column, int value)
{
	//row major ordered
	m.matrix[row + (column * m.column)] = value;
}
void matrixClass_show(struct matrixClass m)
{
	for (int i = 0; i < m.row; i++)
	{
		printf("\t\t|\t");
		for (int j = 0; j < m.column; j++)
		{
			printf("%d\t", m.matrix[i + j*m.row]);
		}
		printf("|\n");
	}
}
void matrixClass_init(struct matrixClass m)
{

	for (int i = 0; i < m.row; i++)
	{
		for (int j = 0; j < m.column; j++)
		{
			m.matrix[i + (j * m.column)] = 0;
		}
	}


}
void * mult_matrix(void *arguments)
{
	struct arg_struct *arg = arguments;

	int from = (arg->slice * ROW_MATRIX1) / num_thread; // note that this 'slicing' works fine
	int to = ((arg->slice + 1) * ROW_MATRIX1) / num_thread; // even if SIZE is not divisible by num_thrd
	int i, j, k;

	//printf("computing slice %d (from row %d to %d)\n", arg->slice, from, to - 1);
	for (int i = from; i < to; i++)
	{
		for (int j = 0; j < arg->m2.column; j++)
		{
			for (int k = 0; k < arg->m1.column; k++)
			{
				//c[i][j] = c[i][j] + (a[i][k] * b[k][j]);
				int rposi = i + (j*arg->m1.column);
				int m1posi = i + (k*arg->m1.column);
				int m2posi = k + (j*arg->m1.column);
				result.matrix[rposi] += arg->m1.matrix[m2posi] * arg->m2.matrix[m1posi];
			}

		}
	}
	//printf("finished slice %d\n", arg->slice);
	return 0;
}

void * mult_vector(void * arguments)
{
	struct arg_struct_vm *arg = arguments;
	struct vectorClass result;
	result.column = (arg->m2.row);
	result.vector = malloc(sizeof(int) *result.column);
	for (int i = 0; i<arg->v1.column; i++)
	{
		result.vector[i] = 0;
	}
	for (int i = 0; i < (arg->m2.row); i++)
	{
		for (int j = 0; j < arg->v1.column; j++)
		{
			result.vector[i] += (arg->m2.matrix[i + (j*arg->m2.column)] * arg->v1.vector[j]);
		}
	}

}

int main()

{

	clock_t t;
	int f;
	t = clock();

	pthread_t* thread;
	thread = (pthread_t*)malloc(num_thread*sizeof(pthread_t));

	srand(time(NULL));
	struct vectorClass v1;
	v1.column = SIZE_VECTOR;
	v1.vector = malloc(sizeof(int) *v1.column);
	vectorClass_init(v1);
	for (int i = 0; i < v1.column; i++)
	{

	vectorClass_allocate(v1, i, rand());
	}
	//printf("Your vector: \n");
	//vectorClass_show(v1);

	struct matrixClass m1;
	m1.column = COLUMN_MATRIX1;
	m1.row = ROW_MATRIX1;
	m1.matrix = malloc(sizeof(int) * m1.column*m1.row);
	matrixClass_init(m1);
	int val = 0;
	for (int i = 0; i < m1.row; i++)
	{
		for (int j = 0; j < m1.column; j++)
		{
			matrixClass_allocate(m1, i, j, val++);
		}
	}
	//printf("Your matrix: \n");
	//matrixClass_show(m1);

	/*		 struct matrixClass m2;
	m2.column = COLUMN_MATRIX2;
	m2.row = ROW_MATRIX2;
	m2.matrix = malloc(sizeof(int) * m2.column*m2.row);
	matrixClass_init(m2);
	val = 0;
	for (int i = 0; i < m2.row; i++)
	{
		for (int j = 0; j < m2.column; j++)
		{
			matrixClass_allocate(m2, i, j, val++);

		}
	}*/
	//printf("Your matrix: \n");
	//matrixClass_show(m2);

	result.row = ROW_MATRIX1;
	result.column = COLUMN_MATRIX1;
	result.matrix = malloc(sizeof(int) * result.column*result.row);
	for (int i = 0; i<result.row* result.column; i++)
	{
		result.matrix[i] = 0;
	}
	struct arg_struct_vm arg;
	arg.v1 = v1;
	arg.m2 = m1;

	// this for loop not entered if threadd number is specified as 1
	for (int i = 1; i < num_thread; i++)
	{
		arg.slice = i;
		// creates each thread working on its own slice of i
		if (pthread_create(&thread[i], NULL, mult_vector, (void *)&arg) != 0)
		{
			perror("Can't create thread");
			free(thread);
			exit(-1);
		}
	}
	struct arg_struct_vm arg1;
	arg1.v1 = v1;
	arg1.m2 = m1;
	arg1.slice = 0;
	mult_vector((void *)&arg1);


	for (int i = 1; i < num_thread; i++)
		pthread_join(thread[i], NULL);
	printf("done\n");
	//matrixClass_show(result);
	free(thread);
	t = clock() - t;
	printf("It took me %d clicks (%f seconds).\n", t, ((float)t) / CLOCKS_PER_SEC);
	return 0;
}
