#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>

#define MAX_FILENAME_LENGTH 256

typedef long int fileSize;

typedef struct
{
    char name[MAX_FILENAME_LENGTH];
    char permissions[11];
    fileSize size;
} FileData;

void FileInfo(const char *fileName, FileData *file)
{
    struct stat fileStat;

    if (stat(fileName, &fileStat) == -1)
    {
        perror("There is no file");
        exit(1);
    }

    strcpy(file->name, fileName);
    snprintf(file->permissions, sizeof(file->permissions), "%o", fileStat.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO));

    file->size = fileStat.st_size;
}

void Merge(int numberOfFiles, char *fileList[], char *archiveName)
{
    fileSize totalFileSize = 0;

    FILE *archive = fopen(archiveName, "w");
    for (int i = 2; i < numberOfFiles - 2; i++)
    {
        FileData file;
        FileInfo(fileList[i], &file);
        totalFileSize += file.size;
    }

    if (!archive)
    {
        perror("Couldn't find the file");
    }
    fprintf(archive, "%010ld", totalFileSize);

    for (int i = 2; i < numberOfFiles - 2; i++)
    {
        FileData dataFile;
        FileInfo(fileList[i], &dataFile);
        printf("|%s, %s, %ld", dataFile.name, dataFile.permissions, dataFile.size);
        fprintf(archive, "|%s, %s, %ld", dataFile.name, dataFile.permissions, dataFile.size);
    }
    printf("|");
    fprintf(archive, "|");

    for (int i = 2; i < numberOfFiles - 2; i++)
    {
        FileData dataFile;
        FILE *openFile = fopen(fileList[i], "r");
        FileInfo(fileList[i], &dataFile);
        int fileSize = dataFile.size;
        char *buffer = (char *)malloc(fileSize + 1);
        fread(buffer, 1, fileSize, openFile);
        strcat(buffer, "\n");
        fwrite(buffer, 1, fileSize, archive);
        fclose(openFile);
        free(buffer);
    }
    fclose(archive);
}

void Extract(char *nameOfArchive, char *nameOfDirection)
{
    DirectoryCreation(nameOfDirection);

    FILE *archive = fopen(nameOfArchive, "r");
    if (!archive)
    {
        perror("Output file is not working");
        return;
    }

    fseek(archive, 0, SEEK_END);
    long fileSize = ftell(archive);
    fseek(archive, 0, SEEK_SET);

    char *b = (char *)malloc(fileSize + 1);
    if (!b)
    {
        perror("Memory allocation error");
        fclose(archive);
        return;
    }

    fread(b, 1, fileSize, archive);
    b[fileSize] = '\0';
    const char delimiters[] = "|,";

    char *t = strtok(b, delimiters);
    FileData infoFile;
    FileData *arrayOfInfoFiles = NULL;
    int infoFileCount = 0;
    int totalFileSize = 0;
    while (t != NULL)
    {
        if (strstr(t, ".txt") != NULL)
        {
            strcpy(infoFile.name, t);
            t = strtok(NULL, delimiters);
            strcpy(infoFile.permissions, t);
            t = strtok(NULL, delimiters);

            if (t != NULL && atoi(t) != 0)
            {
                infoFile.size = atoi(t);
                totalFileSize += infoFile.size;
                char *fileContent = (char *)malloc(infoFile.size);
                if (!fileContent)
                {
                    perror("Memory allocation error");
                    fclose(archive);
                    free(b);
                    return;
                }
                fread(fileContent, 1, infoFile.size, archive);
                free(fileContent);
            }
            arrayOfInfoFiles = realloc(arrayOfInfoFiles, (infoFileCount + 1) * sizeof(FileData));
            arrayOfInfoFiles[infoFileCount++] = infoFile;
        }
        t = strtok(NULL, delimiters);
    }

    int f = fileSize;
    for (int i = infoFileCount - 1; i >= 0; --i)
    {
        f += arrayOfInfoFiles[i].size;
    }

    int n = fileSize;
    for (int i = infoFileCount - 1; i >= 0; i--)
    {
        char *archDir = (char *)malloc(strlen(arrayOfInfoFiles[i].name) + strlen(nameOfDirection) + strlen(nameOfArchive) + 1);
        sprintf(archDir, "%s/%s", nameOfDirection, arrayOfInfoFiles[i].name);

        FILE *destFile = fopen(archDir, "wb");
        int inc = n - arrayOfInfoFiles[i].size;

        for (int j = inc; j < n; j++)
        {
            fseek(archive, j, SEEK_SET);
            fputc(fgetc(archive), destFile);
        }
        n = inc;
        fclose(destFile);
        free(archDir);
    }

    for (int i = infoFileCount - 1; i >= 0; --i)
    {
        printf("%s, ", arrayOfInfoFiles[i].name);
    }

    free(arrayOfInfoFiles);
    free(b);
    fclose(archive);
}

void DirectoryCreation(const char *dirName)
{
    int result = mkdir(dirName, S_IRWXU | S_IRWXG | S_IRWXO);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Usage: %s file1 [file2 ...]\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "-b") == 0)
    {
        char *nameOfOutput = argv[argc - 1];
        Merge(argc, argv, nameOfOutput);
    }
    else if (strcmp(argv[1], "-a") == 0)
    {
        char *nameOfOutput = argv[argc - 2];
        char *nameOfDirection = argv[argc - 1];
        Extract(nameOfOutput, nameOfDirection);
    }

    return 0;
}
