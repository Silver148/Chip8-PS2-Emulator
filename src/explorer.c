#include "explorer.h"

#define MAX_PATH 256
#define MAX_FILES 256

iox_dirent_t dir;

char file_paths[MAX_PATH][MAX_FILES];

int IsCH8()
{
    char *point = strrchr(dir.name, '.');
    if(point != NULL)
    {
        if(strncmp(point + 1, "ch8", sizeof(dir.name)) == 0)
        {
            return 0;
        }else{
            return -1;
        }
    }

    return 0;
}

int ReadDir(char *path)
{
    int ret = fileXioDopen(path);
    int nFiles = 0;

    if(ret < 0)
    {
        printf("Error opening directory: %s\n", path);
        return -1;
    }else{
        printf("%s opened successfully :D\n", path);
    }

    while(fileXioDread(ret, &dir) > 0)
    {
        char file_path[MAX_PATH];

        if(dir.stat.mode & FIO_S_IFREG)
        {
            if(IsCH8() == 0)
            {
                if(strlen("mass0:") + strlen(dir.name) < MAX_PATH)
                {
                    printf("File: %s\n", dir.name);
                    memcpy(file_path, "mass0:", sizeof("mass0:"));
                    strcat(file_path, dir.name);

                    if(nFiles < MAX_FILES)
                    {
                        strcpy(file_paths[nFiles], file_path);
                        nFiles++;
                    }else{
                        printf("The maximum number of ROMs has been reached!\n");
                        break;
                    }
                }
            }
            
        }
        
    }

    /*
    for(int i = 0; i < MAX_FILES; i++) {
        
        if(file_paths[i][0] != '\0')
        {
            printf("ROM %d: %s\n", i, file_paths[i]);
        }

    }
    */

    return 0;
}

char* DeletePrefix(char *str, const char *prefix, char *out)
{
    const char *p = strstr(str, prefix);

    if(p != NULL)
    {
       strcpy(out, p + strlen(prefix)); 
    }

    return out;
}