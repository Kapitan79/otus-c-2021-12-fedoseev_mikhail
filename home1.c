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
    int sigArray[4] = {0x50, 0x4b, 0x05, 0x06};
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

        // \x50\x4b\x03\x04
        short       currSymbol      = 0     ;
        int         currCursorPos   = 0     ;
        _Bool       isArchFind      = false ;

        int         fileNameLen     = 0;
        int         fileNameSymbol  = 0;
        short       archVersion     = 0;

        struct eocdr {
                uint16_t disk_nbr;        /* Number of this disk. */
                uint16_t cd_start_disk;   /* Nbr. of disk with start of the CD. */
                uint16_t disk_cd_entries; /* Nbr. of CD entries on this disk. */
                uint16_t cd_entries;      /* Nbr. of Central Directory entries. */
                uint32_t cd_size;         /* Central Directory size in bytes. */
                uint32_t cd_offset;       /* Central Directory file offset. */
                uint16_t comment_len;     /* Archive comment length. */
                const uint8_t *comment;   /* Archive comment. */
        }eo;

        while(fread(&currSymbol, 1, 1, inputFile))
        {
            if (currSymbol == 0x50)
            {
                if (checkSignature(inputFile, 1))
                {
                    if (!isArchFind)
                    {
                        //isArchFind = true;
                        printf("%s %d\n", "Archive found.", currCursorPos     );

                        fread(&eo, sizeof(eo), 1, inputFile);
                        printf("%d\n", eo.disk_nbr);
                        printf("%d\n", eo.cd_start_disk);
                        printf("%d\n", eo.disk_cd_entries);
                        printf("%d\n", eo.cd_entries);
                        printf("%d\n", eo.cd_size);
                        printf("%d\n", eo.cd_offset);
                        printf("%d\n", eo.comment_len);
                        printf("%d\n", eo.comment);
                    }
                }
                else
                {
                    if (fseek(inputFile, currCursorPos, SEEK_SET))
                    {
                        printf("%s\n", "Error while seek position in file! The program will close.");
                        exit(ERROR_FILE_SEEK);
                    }
                }
            }
            currCursorPos++;
        }

        if (!isArchFind)
        {
            printf("%s%d\n", "Archive not found.", currCursorPos);
        }

        fclose(inputFile);
    }
    return 0;
}
