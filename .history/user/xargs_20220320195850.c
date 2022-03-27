/*
    问题：
        * 命令行里面是怎么处理管道类型的输入的？
            应该就是当做标准输入读进来的
        * read如何处理\n？

*/

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

int buf[1024];

int
main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(2, "usage: xargs command ...");
    }

    char *cmd = argv[1];
    char buf[1024];

    int n = 0, m = 0;
    char *p;
    while ((n = read(0, buf + m, sizeof(buf) - m - 1)))
    {
        p = buf + m;
        m += n;
        buf[m] = '\0';
        char arg_exec[1024];
        strcpy(arg_exec, argv);
    }
}