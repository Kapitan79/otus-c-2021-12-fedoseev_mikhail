#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#define ERROR_OPEN_FILE         1
#define ERROR_FILE_SEEK         2

_Bool checkSignature(FILE *inputFile, short currIndex)
{
    _Bool findFlag = true;
    int sigArray[4] = {0x50, 0x4b, 0x01, 0x02};
    int currVal     = 0;

    for (short arrIndex = currIndex; arrIndex < 4; arrIndex++)
    {
        if (fread(&currVal, 1, 1, inputFile))
        {
            if (sigArray[arrIndex] != currVal)
            {
                findFlag = false;
                break;
            }
        }
    }
    return findFlag;
}


int main(int argc, char *argv[])
{
    if (argc > 0)
    {
        FILE *inputFile = NULL;
        inputFile = fopen(argv[1], "r");

        if (inputFile == NULL)
        {
            printf("File \"%s\" is not open! The program will close.\n", argv[1]);
            exit(ERROR_OPEN_FILE);
        }

        short       currSymbol      = 0     ;
        int         currCursorPos   = 0     ;
        _Bool       isArchFind      = false ;

        while(fread(&currSymbol, 1, 1, inputFile))
        {
            if (currSymbol == 0x50)
            {
                if (checkSignature(inputFile, 1))
                {
                    if (!isArchFind)
                    {
                        isArchFind = true;
                        printf("%s %d\n", "Archive found.", currCursorPos);
                    }

                    char fileName[18] = {0};
                    unsigned int fileNameLen    = 0;

                    fseek(inputFile, currCursorPos+28, SEEK_SET);
                    fread(&fileNameLen, 2, 1, inputFile);

                    fseek(inputFile, currCursorPos+46, SEEK_SET);
                    fread(fileName, fileNameLen, 1, inputFile);
                    //printf("Имя %s Длина имени %d Позиция %d\n", fileName, fileNameLen, currCursorPos+46);
                    printf("Имя файла: ");
                    for (unsigned int charIndex = 0; charIndex < fileNameLen; charIndex++) {
                        printf("%c", fileName[charIndex]);
                    }
                    printf("\n");

                    currCursorPos += 80;
                    if (fseek(inputFile, currCursorPos, SEEK_SET))
                    {
                        printf("Ошибка при сдвиге курсора во время перебора центрального кталога архива.");
                        exit(ERROR_FILE_SEEK);
                    }

                    continue;
                }
                else
                {
                    ++currCursorPos;
                    if (fseek(inputFile, currCursorPos, SEEK_SET))
                    {
                        printf("%s\n", "Ошибка при попытке сдвига курсора после неудачной проверки сигнатуры.");
                        exit(ERROR_FILE_SEEK);
                    }
                    continue;
                }
            }
            ++currCursorPos;
        }

        if (!isArchFind)
        {
            printf("%s%d\n", "Архив не найден.");
        }

        fclose(inputFile);
    }
    return 0;
}
