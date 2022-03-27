/*
    主要工作：
        * 获取命令行参数
        * 根据命令行给出的参数sleep对应计时数
*/
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h" 

int
main(int argc, char *argv[])
{
    char *pattern;
    if (argc != 2)
    {
        fprintf(2, "usage: sleep pattern [number]\n");
        exit(1);
    }
    pattern = argv[1];
    while (*pattern != '\0') 
    {
        if (*pattern >= '0' && *pattern <= '9') pattern++;
        else
        {
            fprintf(2, "usage: make sure you enter a number\n");
        }
    }
    int n = atoi(argv[1]);
    sleep(n);



}