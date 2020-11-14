#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>

enum { success_exit = 0, alloc_err, read_err, arg_err, open_err };

struct matrix
{
    double **m;
    size_t size;
};

struct vector
{
    double *v;
    size_t size;
};

typedef struct matrix matrix_t;
typedef struct vector vector_t;

size_t
maximum(matrix_t *M, size_t row, int *m)
{
    double max = 0;
    size_t maxi = -1;
    size_t count = 0;

	for (count = 0; count < M->size; count++) 
	{
		if (m[count] == -1)
		if (fabs(M->m[count][row]) > max) 
		{
		    max = fabs(M->m[count][row]);
		    maxi = count;
		}
	}
    return maxi;
}

void
*check_malloc(size_t size)
{
    void *mem = calloc(1, size);
    if (mem == NULL)
    {
        perror("malloc");
        exit(alloc_err);
    }
    return mem;
}   

void
check_read(int fd, void *buf, size_t count)
{
    if (read(fd, buf, count) != count)
    {
        perror("read");
        exit(read_err);
    }
}

matrix_t
*read_matrix_fd(int fd)
{
    size_t size, i;
    char *key = "MATRIX";
    char read_key[7];
    matrix_t *matrix = NULL;
    read_key[6] = '\0';
    check_read(fd, read_key, 6);
    if (strncmp(read_key, key, 6))
    {
        puts("This file is not matrix");
        exit(arg_err);
    }

    check_read(fd, &size, sizeof(size_t));
    matrix = check_malloc(sizeof(matrix_t));
    matrix->m = check_malloc(size * sizeof(double*));
    for (i = 0; i < size; ++i)
    {
        matrix->m[i] = check_malloc(size * sizeof(double));
        check_read(fd, matrix->m[i], size * sizeof(double));
    }

    matrix->size = size;
    return matrix;
}

vector_t
*read_vector_fd(int fd, size_t size)
{
    vector_t *vector = check_malloc(sizeof(vector_t));
    vector->v = check_malloc(size * sizeof(double));
    check_read(fd, vector->v, size * sizeof(double));
    vector->size = size;
    return vector;
}

void
show_matrix(matrix_t *matrix, char ch)
{
    size_t i, j, fopened = 0;
    FILE *f = stdout;
    if (matrix->size >= 10)
    {
        f = fopen("LU_output", "a");
        if (f == NULL)
        {
            perror("fopen");
            exit(open_err);
        }

        fopened = 1;
    }

    for (i = 0 ; i < matrix->size; ++i)
    {
        for (j = 0; j < matrix->size; ++j)
            fprintf(f,"%c[%zu][%zu] = %lf\t", ch, i, j, matrix->m[i][j]);
        putc('\n', f);
    }

    if (fopened) fclose(f);
}

void
show_matrixL(matrix_t *matrix, char ch, long *max)
{
    size_t i, k, fopened = 0;
    FILE *f = stdout;
    if (matrix->size >= 10)
    {
        f = fopen("LU_output", "a");
        if (f == NULL)
        {
            perror("fopen");
            exit(open_err);
        }

        fopened = 1;
    }

    for (i = 0; i < matrix->size; ++i)
    {
        for (k = 0; k < matrix->size; ++k)
            if (k <= i)
            fprintf(f,"%c[%zu][%zu] = %lf\t", ch, i, k, matrix->m[max[i]][k]);
        else     
               fprintf(f,"%c[%zu][%zu] = 0.000000\t", ch, i, k);
        putc('\n', f);
    }

    if (fopened) fclose(f);
      putc('\n', f);
}

void
show_matrixU(matrix_t *matrix, char ch, long * max)
{
    size_t i, k, fopened = 0;
    FILE *f = stdout;
    if (matrix->size >= 10)
    {
        f = fopen("LU_output", "a");
        if (f == NULL)
        {
            perror("fopen");
            exit(open_err);
        }

        fopened = 1;
    }

    for (i = 0; i < matrix->size; ++i)
    {
        for (k = 0; k < matrix->size; ++k)
            if (k >= i)
            fprintf(f,"%c[%zu][%zu] = %lf\t", ch, i, k, matrix->m[max[i]][k]);
            else     
               fprintf(f, "%c[%zu][%zu] = 0.000000\t", ch, i, k);
        putc('\n', f);
    }

    if (fopened) fclose(f);
}

void
show_vector(vector_t *vector, char ch)
{
    size_t i, fopened = 0;
    FILE *f = stdout;

    if (vector->size >= 10)
    {   
        f=fopen("LU_output", "a");
        if (f == NULL)
        {
            perror("fopen");
            exit(open_err);    
        }

        fopened = 1;
    }

    for (i = 0; i < vector->size; ++i)
        fprintf(f, "%c[%zu] = %lf\n", ch, i, vector->v[i]);
    if (fopened) fclose(f);
}

void
free_matrix(matrix_t *matrix)
{
    size_t i;
    
    for (i = 0; i < matrix->size; ++i)
        free(matrix->m[i]);
    free(matrix->m);
    free(matrix);
}

void
free_vector(vector_t *vector)
{
    free(vector->v);
    free(vector);
}

long
*LU_decomposition(matrix_t *L)
{
    size_t i, j, k, swp, n = L->size;
    double sum = 0;

    long *max = calloc(n, sizeof(long));
    int *u_was_counted=calloc(n, sizeof(int));

    for(size_t c = 0; c < n; ++c)
    {
        max[c] = c;
        u_was_counted[c] = -1;
    }

    swp = max[0];
    max[0] = maximum(L, 0, u_was_counted);
    max[maximum(L, 0, u_was_counted)] = swp;
    u_was_counted[maximum(L, 0, u_was_counted)] = 1;

    for(k = 1; k < n; ++k)
        L->m[max[0]][k] /= L->m[max[0]][0];

    for (k = 1; k < n; ++k)
    {
        for (i = k; i < n; ++i)
        {
            for (j = 0; j < k; ++j)
            {
                sum += (L->m[max[i]][j] * L->m[max[j]][k]);
            }
            L->m[max[i]][k] -= sum;
            sum = 0;
        }

            swp = maximum(L, k, u_was_counted);
            for (size_t count = 0; count < n; ++count)
            	if(max[count] == swp)
            		max[count] = max[k];
    	max[k] = swp;
   		u_was_counted[swp] = 1;

   	    for(i = k + 1; i < n; ++i)
   	    {
            if(L->m[max[k]][max[k]] == 0)
            {
                perror("ERROR:div0\n");
                exit(15);
            }

            for (j = 0; j < k; ++j)
            {
           	    sum += L->m[max[k]][j] * L->m[max[j]][i];
            }
            L->m[max[k]][i] = (L->m[max[k]][i] - sum) / L->m[max[k]][k];
            sum = 0;
        }
    }

    free(u_was_counted);
    return max;
}

void
check_det(matrix_t *M)
{
    double p = 1;
    size_t i;

    for (i = 0; i < M->size; ++i)
    {
        p *= M->m[i][i];
    }

    if (p == 0)
    {
        puts("Det(A) = 0");
        exit(success_exit);
    }
}

void
Ly_b(matrix_t *L, vector_t *y, vector_t *b, long * max)
{
    size_t i, k;
    double summ = 0;

    for (i = 0; i < L->size; ++i)
    {
        for (k = 0, summ = 0; k <= i; ++k)
        {
            summ += L->m[max[i]][k] * y->v[k];
        } 
        y->v[i] = (b->v[max[i]] - summ) / L->m[max[i]][i];
    }    
}

void
Ux_y(matrix_t *A, vector_t *x, vector_t *y, long*  max)
{
    int i, k;
    double summ;
    for(i = 0; i < A->size; ++i)
        A->m[max[i]][i] = 1.000000;
    for (i = A->size - 1; i >= 0; --i)
    {
        for (k = A->size - 1, summ = 0; k >= i; --k)
        {
             summ += A->m[max[i]][k] * x->v[k];
        }   

        x->v[i] = (y->v[i] - summ);
    }
}

double
norma_vector(vector_t *vector)
{
    size_t i;
    double summ = 0;

    for (i = 0; i < vector->size; ++i)
        summ += vector->v[i] * vector->v[i];

    return sqrt(summ);    
}

double
residual(matrix_t *L, vector_t *x, vector_t *b, int fd)
{
    size_t i, j;
    double norma;
    free_matrix(L);
    lseek(fd,0,SEEK_SET);
      L = read_matrix_fd(fd);

    vector_t *Lx = calloc(1, sizeof(vector_t));
    Lx->v = calloc(x->size, sizeof(double));
    Lx->size=x->size;
    
    for (i = 0; i < x->size; ++i)
        Lx->v[i] = 0;
    
    for (i = 0; i < L->size; ++i)
    {
        for (j = 0; j < L->size; ++j)
        {
            Lx->v[i] += L->m[i][j] * x->v[j];
        }
    }
    
    for (i = 0; i < Lx->size; ++i)
        Lx->v[i] -= b->v[i];

    norma = norma_vector(Lx);
    
    free_vector(Lx);

    return norma;
}


int
main(int argc, char **argv)
{
    if (argc != 2)
	{
        fputs("Usage: ./LU <namepath>", stderr);
        return arg_err;
	}

    int fd;
    vector_t *b = NULL;
    vector_t *y = calloc(1, sizeof(vector_t)), *x = calloc(1, sizeof(vector_t));
    matrix_t *L = NULL;
    

    fd = open(argv[1], O_RDONLY);
    if (fd == -1)
    {
        perror("open");
        return open_err;
    }

    L = read_matrix_fd(fd);
    b = read_vector_fd(fd, L->size);
    lseek(fd, 0 ,SEEK_SET);
    long* permutation = LU_decomposition(L);
    check_det(L);
    y->v = check_malloc(L->size * sizeof(double));
    y->size = L->size;
    x->v = check_malloc(L->size * sizeof(double));
    x->size = L->size;
    Ly_b(L, y, b, permutation);
    Ux_y(L, x, y, permutation);
    free_vector(y);
    show_matrix(L, 'A');
    show_vector(b, 'b');
    show_vector(x, 'x'); 
     printf("RESIDUAL: %e\n", residual(L, x, b,  fd));
    free_vector(b);
    free_vector(x);
    free(permutation);

    return success_exit;
}
