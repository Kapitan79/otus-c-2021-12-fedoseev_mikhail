#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <wchar.h>

#include <arpa/inet.h>

/* Ключи запуска */
#define INPUT_FILE  1
#define CODE_PAGE   2
#define OUTPUT_FILE 3

/* Ошибки выхода */
#define FILE_NOT_OPEN    1
#define FILE_NOT_REWRITE 2
#define BAD_CODE_PAGE    3
#define BAD_PARAMETERS   4

/*Количество букв в двух диапазонах маленьких русских букв UTF-8*/
#define UTF_AREA_1 16
#define UTF_AREA_2 16
#define FULL_CODE_PAGE_AREA 32

typedef unsigned short ushort;

int utf8Areas       [5]  = {53392,    53424,  53632,   53377,   53649};  //"нулевые" имдексы для больших русских букв,потом 2 "нулевых" индекса малых русских букв, а также Ё и ё
int koi8Areas       [4]  = {0,       32,     65,      66};              //"нулевые" индексы для больших и малых букв (массив koi8RusArea), а также Ё и ё
int iso_8859_5Areas [4]  = {176,     208,    161,     241};             //"нулевые" индексы для больших и малых букв, а также Ё и ё
int cp_1251Areas    [4]  = {192,     224,    168,     184};             //"нулевые" индексы для больших и малых букв, а также Ё и ё

const int koi8RusArea [66] = {225,226,247,231,228,229,246,250,233,234,235,236,237,238,239,240,242,243,244,245,230,232,227,254,251,253,225,249,248,252,224,241,
    193, 194, 215, 199, 196, 197, 214, 218, 201,202,203,204,205,206,207,208,210,211,212,213,198,200,195,222,219,221,223,217,216,220,192,209,179,163}; // коды в алфавитном порядке

bool checkReWrite()
{
    char q = 0;
    scanf("%c",&q);

    bool isRewrite = false;
    if (q == 'Y' || q == 'y')
    {
        isRewrite = true;
    }
    else if (q == 'N' || q == 'n')
    {
        isRewrite = false;
    }
    else
    {
        if(q != 10)printf("Вы ввели не корректное значение. Лучше введите \"Y\" или \"N\". Или задолбаю.\n");
        isRewrite = checkReWrite();
    }
    return isRewrite;
}

ushort convertFrom(char* codePage, int inputCode)
{
    ushort utfCode = 0;
    int currCodePage[4] = {0};

    if(strcmp(codePage, "cp-1251") == 0)
    {
        memcpy(currCodePage, cp_1251Areas, sizeof(currCodePage));
    }
    else if(strcmp(codePage, "koi8") == 0)
    {
        memcpy(currCodePage, koi8Areas, sizeof(currCodePage));
        bool isRusCode = false;
        for(int areaIndex = 0; areaIndex < 66; areaIndex++)
        {
            if (koi8RusArea[areaIndex] == inputCode)
            {
                inputCode = areaIndex;
                isRusCode = true;
            }
        }
        if (!isRusCode)
        {
            return inputCode;
        }
    }
    else if(strcmp(codePage, "iso-8859-5") == 0)
    {
        memcpy(currCodePage, iso_8859_5Areas, sizeof(currCodePage));
    }

    if (inputCode >= currCodePage[0] && inputCode < currCodePage[0]+FULL_CODE_PAGE_AREA)
    {
        utfCode = utf8Areas[0] + inputCode - currCodePage[0];

    }
    else if (inputCode >= currCodePage[1] && inputCode < currCodePage[1]+UTF_AREA_1)
    {
        utfCode = utf8Areas[1] + inputCode - currCodePage[1];
    }
    else if (inputCode >= currCodePage[1]+UTF_AREA_1 && inputCode < currCodePage[1]+UTF_AREA_2+UTF_AREA_1)
    {
        utfCode = utf8Areas[2] + inputCode - currCodePage[1] - UTF_AREA_1;
    }
    else if (inputCode == currCodePage[2] )
    {
        utfCode = utf8Areas[3];
    }
    else if (inputCode == currCodePage[3] )
    {
        utfCode = utf8Areas[4];
    }
    else
    {
        return inputCode;
    }
    return utfCode;
}

int main(int argc, char *argv[])
{
    FILE *inputFile  = NULL;
    FILE *outputFile = NULL;
    FILE *checkFile  = NULL;

    if(argc == 4)
    {
        printf("Конвертация текстового файла в кодировку UTF8.\nПопытка перекодировки содержимого файла %s  из кодировки %s и записи результата в файл %s\n",
               argv[INPUT_FILE], argv[CODE_PAGE], argv[OUTPUT_FILE]);

        if(strcmp(argv[CODE_PAGE],"cp-1251") != 0 && strcmp(argv[CODE_PAGE],"koi8") != 0 && strcmp(argv[CODE_PAGE],"iso-8859-5") != 0)
        {
            printf("Введено не верное имя кодировки.\nДопустимые типы кодировки cp-1251, koi8, iso-8859-5\n");
            exit(BAD_CODE_PAGE);
        }

        inputFile = fopen(argv[INPUT_FILE], "r");

        if (inputFile == NULL)
        {
            printf("Файл \"%s\" не открылся! Программа будет закрыта.\n", argv[INPUT_FILE]);
            exit(FILE_NOT_OPEN);
        }

        checkFile = fopen(argv[OUTPUT_FILE], "r");

        if (checkFile == NULL)
        {
            outputFile = fopen(argv[OUTPUT_FILE], "w");
        }
        else
        {
            fclose(checkFile);
            printf("Файл \"%s\" уже существует! Перезаписать? (Y/N)\nВ случае отказа программа будет закрыта. \n", argv[OUTPUT_FILE]);
            if(checkReWrite())
            {
                outputFile = fopen(argv[OUTPUT_FILE], "w");

                if (inputFile == NULL)
                {
                    printf("Файл \"%s\" создать\\перезаписать не удалось! Программа будет закрыта.\n", argv[OUTPUT_FILE]);
                    exit(FILE_NOT_OPEN);
                }
            }
            else
            {
                fclose(inputFile);
                exit(FILE_NOT_REWRITE);
            }
        }
        int currSymbol = 0;
        ushort convSymbol = 0;
        while(fread(&currSymbol, 1, 1, inputFile))
        {
            convSymbol = htons(convertFrom(argv[CODE_PAGE], currSymbol));
            fwrite(&convSymbol, sizeof(ushort), 1, outputFile);
        }
    }
    else
    {
        printf("Введите все необходимые параметры для конвертации кодировки (имя исходного файла, кодировку и имя выходного файла через пробел).\nТипы кодировки: cp-1251, koi8, iso-8859-5.\n");
        exit(BAD_PARAMETERS);
    }

    fclose(inputFile);
    fclose(outputFile);

    return 0;
}
