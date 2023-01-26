#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>


struct arr_wrapper {
    int32_t *arr;
    int32_t size;
};
struct Data {
    int32_t **chunks;
    int32_t *chunk_sizes;
    int32_t n_chunks;
    struct arr_wrapper arr_wrp;
};

void qsortRecursive(int32_t *mas, int32_t size);
struct arr_wrapper bond(struct arr_wrapper arr_wrp_1, struct arr_wrapper arr_wrp_2);
void* sort_array_wrapper(void *param);


long get_time_us() {
    struct timeval tval;
    gettimeofday(&tval, NULL);
    return (1.0e6 * tval.tv_sec + tval.tv_usec);
}


struct arr_wrapper sort_array(int32_t **chunks, int32_t *chunk_sizes, int32_t n_chunks) {
    if (n_chunks == 1) {
        struct arr_wrapper ret = {chunks[0], chunk_sizes[0]};
        qsortRecursive(ret.arr, ret.size);
        return ret;
    }
    int32_t mid_chunk = n_chunks / 2;
    int32_t **chunks_l = chunks;
    int32_t **chunks_r = &chunks[mid_chunk];

    int32_t *chunk_sizes_l = chunk_sizes;
    int32_t *chunk_sizes_r = &chunk_sizes[mid_chunk];

    int32_t n_chunks_l = mid_chunk;
    int32_t n_chunks_r = n_chunks - mid_chunk;

    int32_t elem_total = 0;
    for (int32_t i = 0; i < n_chunks; i++) elem_total += chunk_sizes[i];

    struct Data param_l = {chunks_l, chunk_sizes_l, n_chunks_l};
    struct Data param_r = {chunks_r, chunk_sizes_r, n_chunks_r};

    pthread_t thread_l, thread_r;
    pthread_create(&thread_l, NULL, &sort_array_wrapper, (void *)&param_l);
    pthread_create(&thread_r, NULL, &sort_array_wrapper, (void *)&param_r);

    pthread_join(thread_l, NULL);
    pthread_join(thread_r, NULL);

    return bond(param_l.arr_wrp, param_r.arr_wrp);
}


void* sort_array_wrapper(void *param) {
    struct Data *input = (struct Data*)param;
    input -> arr_wrp = sort_array(
        input -> chunks,
        input -> chunk_sizes,
        input -> n_chunks
    );
    return 0;
}


void qsortRecursive(int32_t *mas, int32_t size) {
    int32_t i = 0, j = size - 1;
    int32_t mid = mas[size / 2];

    do {
        while(mas[i] < mid) i++;
        while(mas[j] > mid) j--;

        if (i <= j) {
            int32_t tmp = mas[i];
            mas[i] = mas[j];
            mas[j] = tmp;

            i++; j--;
        }
    } while (i <= j);

    if(j > 0) qsortRecursive(mas, j + 1);
    if (i < size) qsortRecursive(&mas[i], size - i);
}


struct arr_wrapper bond(struct arr_wrapper arr_wrp_1, struct arr_wrapper arr_wrp_2) {
    int32_t *mas1 = arr_wrp_1.arr;
    int32_t size1 = arr_wrp_1.size;
    int32_t *mas2 = arr_wrp_2.arr;
    int32_t size2 = arr_wrp_2.size;

    int32_t *arr = (int32_t*)malloc((size1+size2)*sizeof(int32_t));
    int32_t ind1 = 0, ind2 = 0, ind = 0;

    while (ind1 < size1 || ind2 < size2) {
        if (ind1 >= size1 && ind2 < size2) {arr[ind++] = mas2[ind2++]; continue;}
        if (ind1 < size1 && ind2 >= size2) {arr[ind++] = mas1[ind1++]; continue;}

        if (mas1[ind1] > mas2[ind2]) arr[ind++] = mas2[ind2++];
        else arr[ind++] = mas1[ind1++];
    }

    struct arr_wrapper res = {arr, size1+size2};
    return res;
}

int main(int argc, char **argv) {
    if (argc < 4) {
        return 1;
    }
    FILE* inFile = fopen (argv[1] , "rb");
    FILE* outFile = fopen (argv[2] , "wb");
    int nChunks = strtol(argv[3], NULL, 10);

    int32_t inputSize; fread(&inputSize, sizeof(inputSize), 1, inFile);
    int32_t *inputArray = (int32_t*)malloc(inputSize*sizeof(int32_t));
    fread(inputArray, sizeof(*inputArray), inputSize, inFile);

    int32_t *chunk_sizes = (int32_t*)malloc(inputSize*sizeof(int32_t));
    int32_t **chunked_array = (int32_t**)malloc(nChunks*sizeof(int32_t*));

    int32_t div = inputSize / nChunks;
    int32_t cursor = 0;

    for (int32_t i = 0; i < nChunks; i++) {
        chunk_sizes[i] = i < inputSize % nChunks ? div + 1 : div;
        chunked_array[i] = inputArray + cursor;
        cursor += chunk_sizes[i];
    }

    long t0 = get_time_us();
    struct arr_wrapper out = sort_array(chunked_array, chunk_sizes, nChunks);
    printf("Elapsed time(us):\t%ld\n", get_time_us() - t0);

    int32_t *out_arr = out.arr;
    int32_t size = out.size;

    fwrite(&size, sizeof(size), 1, outFile);
    fwrite(out_arr, sizeof(int32_t), size, outFile);

    free(inputArray);
    free(chunk_sizes);
    free(chunked_array);

    fclose(inFile);
    fclose(outFile);
}