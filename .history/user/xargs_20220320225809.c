/*
    问题：
        * 命令行里面是怎么处理管道类型的输入的？
            应该就是当做标准输入读进来的
        * read如何处理\n？
    思路：
        * 由于read似乎不会遇到\n而停止，所以得一个字符一个字符去读入
        * 对于读入的行，需要进行将行处理成字符串数组，其实就是给一个指针
        * 字符串数组也是以0标志结束的

*/

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

int buf[1024];

int
readline(char *exec_argv, int len)
{
    int n = 0;
    // 按字节读取一行
    while (read(0, buf + n, 1))
    {
        n++;
        if (n > 1024)
        {
            fprintf(2, "args too long!\n");
            exit(1);
        }
        if (buf[n] == '\n') break;
    }
    if (n == 0) return 0;
    buf[n] = '\0';

    int i = 0;
    while (buf[i] == ' ' && i < n) i++;
    while (i < n)
    {   
        exec_argv[len++] = buf + i;
        while (buf[i] != ' ' && i < n) i++;
        buf[i] = '\0', i++;
        while (buf[i] == ' ' && i < n) i++;
    }
    return len;
}

int
main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(2, "usage: xargs command ...");
    }

    char *cmd = argv[1];

    char *exec_argv[MAXARG];

    for (int i = 1; i < argc; i++) exec_argv[i - 1] = argv[i];

    int exec_argc = 0;
    while ((exec_argc = readline(exec_argv, argc - 1)) != 0)
    {
        exec_argv[exec_argc] = '\0';

        int pid = fork();
        if (pid == 0)
        {
            exec(cmd, exec_argv);
            fprintf(2, "exec fail\n");
            exit(1);
        }
        else
        {
            wait((int *) 0);
        }
    }
}