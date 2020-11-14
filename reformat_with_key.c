#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

enum { success_exit = 0, alloc_err, write_err, arg_err, open_err };

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

void *
check_malloc(size_t size)
{
    void *mem = malloc(size);
    if (mem == NULL)
    {
        perror("malloc");
        exit(alloc_err);
    }
    return mem;
}

void
check_write(int fd, const void *buf, size_t count)
{
    if (write(fd, buf, count) != count)
    {
        perror("write");
        exit(write_err);
    }
}

matrix_t *
read_matrix_f(FILE *f)
{
    size_t size, i, j;
    matrix_t *matrix = NULL;

    fscanf(f, "%zu", &size);

    matrix = check_malloc(sizeof(matrix_t));
    matrix->m = check_malloc(size * sizeof(double*));
    
    for (i = 0; i < size; ++i)
        matrix->m[i] = check_malloc(size * sizeof(double));

    for (i = 0; i < size; ++i)
        for (j = 0; j < size; ++j)
        {
            
            fscanf(f, "%lf", &matrix->m[i][j]);
        }
    
    matrix->size = size;
    
    return matrix;
}

vector_t *
read_vector_f(FILE *f, size_t size)
{
    size_t i;
    vector_t *vector = check_malloc(sizeof(vector_t));
    vector->v = check_malloc(size * sizeof(double));
    
    for (i = 0; i < size; ++i)
    {
        fscanf(f, "%lf", &vector->v[i]);
    }

    vector->size = size;

    return vector;
}

void
write_matrix(int fd, matrix_t *matrix)
{
    size_t i;
    char *key = "MATRIX";

    check_write(fd, key, 6);

    check_write(fd, &matrix->size, sizeof(size_t));

    for (i = 0; i < matrix->size; ++i)
        check_write(fd, matrix->m[i], matrix->size * sizeof(double));
        
}

void
write_vector(int fd, vector_t *vector)
{
    check_write(fd, vector->v, vector->size * sizeof(double));
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

int
main(int argc, char **argv)
{
    int fd;
    matrix_t *matrix = NULL;
    vector_t *vector = NULL;

    if (argc != 2)
    {
        fputs("Usage: ./gen_matrix <namepath>\n", stderr);
        return arg_err;
    }

    fd = open(argv[1], O_WRONLY | O_CREAT, 0666);
    if (fd == -1)
    {
        perror("open");
        return open_err; 
    }
    FILE * file = fopen("input.txt","r+");
    matrix = read_matrix_f(file);
    vector = read_vector_f(file, matrix->size);
    
    write_matrix(fd, matrix);
    write_vector(fd, vector);
    
    free_matrix(matrix);
    free_vector(vector);
fclose(file);
close(fd);
    return success_exit;
}
