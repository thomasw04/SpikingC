#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <getopt.h>

#include "config.h"

#define MAX_LINE_LENGTH 10000 // Adjust based on your CSV line length
#define BUFFER_SIZE 30055 // ?????????

// Function to read CSV file into a 2D array
float **readCSV(const char *filename, int *rows, int *cols)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        fprintf(stderr, "Failed to open file %s: %s\n", filename, strerror(errno));
        return NULL;
    }

    char line[MAX_LINE_LENGTH];
    if (!fgets(line, MAX_LINE_LENGTH, file))
    {
        // Handle error or empty file
        fclose(file);
        return NULL;
    }

    // Temporarily count columns
    int colCount = 1; // Starting at one since counting separators
    for (char *temp = line; *temp; temp++)
    {
        if (*temp == ',')
            colCount++;
    }

    *cols = colCount; // Assuming cols is correctly passed in

    float **data = NULL;
    *rows = 0;

    rewind(file); // Go back to start to read data again

    while (fgets(line, MAX_LINE_LENGTH, file))
    {
        data = (float **)realloc(data, (*rows + 1) * sizeof(float *));
        if (!data)
        {
            // Handle realloc failure
            *rows = 0;
            fclose(file);
            return NULL;
        }

        data[*rows] = (float*)malloc(colCount * sizeof(float));
        if (!data[*rows])
        {
            // Handle malloc failure, cleanup previously allocated rows
            for (int i = 0; i < *rows; i++)
                free(data[i]);
            free(data);
            *rows = 0;
            fclose(file);
            return NULL;
        }

        // Split line into tokens and convert to float
        char *token = strtok(line, ",");
        int col = 0;
        while (token != NULL && col < colCount)
        {
            data[*rows][col++] = atof(token);
            token = strtok(NULL, ",");
        }
        (*rows)++;
    }

    fclose(file);
    return data;
}

void freeCSVData(float **data, int rows)
{
    for (int i = 0; i < rows; i++)
    {
        free(data[i]);
    }
    free(data);
}

long open_csv(const char* filepath, FILE** out_file, char** buffer, size_t* elements)
{
    FILE *file = fopen(filepath, "r");
    if (!file)
    {
        fprintf(stderr, "Failed to open file %s: %s\n", filepath, strerror(errno));
        return -errno;
    }

    size_t size = fseek(file, 0, SEEK_END);
    rewind(file);

    *buffer = (char*)malloc(size);
    size_t ret = fread(*buffer, 1, size, file);

    if (ret != size)
    {
        if(ferror(file))
        {
            fprintf(stderr, "Failed to read file %s: %s\n", filepath, strerror(errno));
            fclose(file);
            return -errno;
        }
        
        fprintf(stderr, "Failed to read file %s: Unexpected EOF.\n", filepath);
        fclose(file);
        return -EIO;
    }

    size_t count = 0;
    for (size_t i = 0; i < size; i++)
    {
        //We don't handle \r\n line endings for now.
        if (*buffer[i] == ',' || *buffer[i] == '\n')
            count++;
    }

    *elements = count;
    *out_file = file;
    return size;
}


long f32_array_load(char* src, size_t src_size, wfloat_t* dest, size_t size)
{
    size_t index = 0;
    size_t begin = 0;
    for(size_t i = 0; i < src_size; i++)
    {
        if (src[i] == ',' || src[i] == '\n')
        {
            src[i] = '\0';
            float val = strtof(src + begin, NULL);

            if(val == 0.0f) {
                fprintf(stderr, "Failed to convert string to float: %s\n", src + begin);
                return -EINVAL;
            }

            if (errno == ERANGE)
            {
                fprintf(stderr, "Out of range error: %s\n", src + begin);
                return -ERANGE;
            }

            dest[index++] = val;

            if(index >= size)
            {
                fprintf(stderr, "Too many elements in file.\n");
                return -E2BIG;
            }

            begin = i + 1;
        }
    }

    return index;
}

int f32_array_store(const char* filepath, wfloat_t* data, size_t size, bool is_target_le)
{
    FILE *file = fopen(filepath, "w");
    if (!file)
    {
        fprintf(stderr, "Failed to open file %s: %s\n", filepath, strerror(errno));
        return -errno;
    }

    //Write raw bytes to file with the correct endianess.
    int num = 1;
    bool swap = (*(char *)&num == 1) != is_target_le;

    for (size_t i = 0; i < size; i++)
    {
        if (swap)
        {
            wfloat_t val = data[i];
            for (size_t j = 0; j < sizeof(wfloat_t); j++)
            {
                fputc(((char*)&val)[sizeof(wfloat_t) - j - 1], file);
            }
        }
        else
        {
            fwrite(&data[i], sizeof(wfloat_t), 1, file);
        }
    }

    fclose(file);
    return 0;
}


int main(int argc, char* argv[])
{
    int opt;

    char *output_file = NULL;
    bool target_le = false;

    while ((opt = getopt(argc, argv, "o:l")) != -1)
    {
        switch (opt)
        {
            case 'l':
                target_le = true;
                break;
            case 'o':
                output_file = optarg;
                break;
            default:
                fprintf(stderr, "Usage: %s [input...] [-o output]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (!output_file)
    {
        fprintf(stderr, "No output file given.\n");
        exit(EXIT_FAILURE);
    }

    // Load all the files that were given
    FILE* file;
    if(optind < 0 || optind >= argc)
    {
        fprintf(stderr, "No input files given.\n");
        exit(EXIT_FAILURE);
    }

    char* buffer[argc - optind];
    size_t sizes[argc - optind];
    size_t total_elements = 0;
    for (int i = optind; i < argc; i++)
    {
        size_t elements = 0;
        long size = open_csv(argv[i], &file, &buffer[i - optind], &elements);

        if (size < 0)
        {
            exit(EXIT_FAILURE);
        }

        sizes[i - optind] = size;
        total_elements += elements;
        printf("load %s\n", argv[i]);
    }

    // Allocate memory for the data
    wfloat_t* data = (wfloat_t*)malloc(total_elements * sizeof(wfloat_t));

    // Load the data into the array
    size_t index = 0;
    for (int i = optind; i < argc; i++)
    {
        long ret = f32_array_load(buffer[i - optind], sizes[i - optind], data + index, total_elements - index);

        if (ret != (long)(total_elements - index))
        {
            exit(EXIT_FAILURE);
        }

        index += ret;
    }

    // Store the data in a file
    int ret = f32_array_store(output_file, data, total_elements, target_le);

    if (ret != 0)
    {
        exit(EXIT_FAILURE);
    }
    return 0;
}