#include <stdio.h>
#include <stdbool.h>
#include <errno.h>

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

int load_float_list(const char* filepath, wfloat_t** data)
{
    FILE *file = fopen(filepath, "r");
    if (!file)
    {
        fprintf(stderr, "Failed to open file %s: %s\n", filepath, strerror(errno));
        return -errno;
    }

    size_t size = fseek(file, 0, SEEK_END);
    rewind(file);

    char buffer[size];
    fread(buffer, 1, size, file);

    size_t count = 0;
    for (size_t i = 0; i < size; i++)
    {
        //We don't handle \r\n line endings for now.
        if (buffer[i] == ',' || buffer[i] == '\n')
            count++;
    }

    // Assuming data is already allocated.
    *data = (wfloat_t*)malloc(count * sizeof(wfloat_t));

    if (!*data)
    {
        // Handle malloc failure
        fclose(file);
        return -ENOMEM;
    }

    size_t index = 0;
    size_t begin = 0;
    for(size_t i = 0; i < size; i++)
    {
        if (buffer[i] == ',' || buffer[i] == '\n')
        {
            buffer[i] = '\0';
            float val = strtof(buffer + begin, NULL);

            if(val == 0.0f) {
                fprintf(stderr, "Failed to convert string to float: %s\n", buffer + begin);
                free(*data);
                fclose(file);
                return -EINVAL;
            }

            if (errno == ERANGE)
            {
                fprintf(stderr, "Out of range error: %s\n", buffer + begin);
                free(*data);
                fclose(file);
                return -ERANGE;
            }

            (*data)[index++] = val;
            begin = i + 1;
        }
    }

    return count;
}

bool is_little_endian()
{
    int num = 1;
    return (*(char *)&num == 1);
}

int serialize_float_list(const char* filepath, wfloat_t* data, size_t size, bool little_endian)
{
    FILE *file = fopen(filepath, "w");
    if (!file)
    {
        fprintf(stderr, "Failed to open file %s: %s\n", filepath, strerror(errno));
        return -errno;
    }

    //Write raw bytes to file with the correct endianess.
    bool swap = is_little_endian() != little_endian;

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



void loadStaticWeightsAndBiases(void)
{
    // Adjust file paths and array indices as needed
    loadCSVToStaticWeightArray(PATH_WEIGHTS_FC1, W, 0, INPUT_SIZE * L1_SIZE_OUT);
    loadCSVToStaticBiasArray(PATH_BIAS_FC1, B, 0, L1_SIZE_OUT);

    // Calculate start index for each subsequent layer based on the previous layers' sizes
    unsigned int wIdx2 = INPUT_SIZE * L1_SIZE_OUT;
    unsigned int bIdx2 = L1_SIZE_OUT;

    loadCSVToStaticWeightArray(PATH_WEIGHTS_FC2, W, wIdx2, LIF1_SIZE * L2_SIZE_OUT);
    loadCSVToStaticBiasArray(PATH_BIAS_FC2, B, bIdx2, L2_SIZE_OUT);

    // And so on for each layer, adjusting the indices accordingly
    unsigned int wIdx3 = wIdx2 + LIF1_SIZE * L2_SIZE_OUT;
    unsigned int bIdx3 = bIdx2 + L2_SIZE_OUT;

    loadCSVToStaticWeightArray(PATH_WEIGHTS_FC3, W, wIdx3, LIF2_SIZE * L3_SIZE_OUT);
    loadCSVToStaticBiasArray(PATH_BIAS_FC3, B, bIdx3, L3_SIZE_OUT);
}


int main() {
    

    printf("Hello, World!\n");
    return 0;
}