/*
    主要用到的系统调用
    * int read(int, void*, int);
    * int write(int, const void*, int);
    * int pipe(int*);
    * int fork(void);
    * int wait(int*);
    
    主要思路：
    1. 理解什么是质数筛 https://zhuanlan.zhihu.com/p/100051075
    2. 加速的关键在于，我们在每个质数去找他的非倍数的数时，去同时处理每个质数的筛选过程
    3. 想要同时实现这种处理，就需要多进程了，并且使用管道来作为通信方式
    4. 其实作业里面那张图已经写得很明白了
    一些问题
    1. 如何关闭不需要的文件描述符
        还是按照左右管道的方式去思考
    2. 如何wait结束的子进程

*/

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void
is_primes(int *p)
{
    // 关闭左边管道写端
    close(p[1]);
    int prime = 0;
    int n = read(p[0], &prime, 4);
    if (!n) exit(0);
    // fprintf(1, "pid: %d ", getpid());
    if (n != 4) 
    {
        fprintf(2, "input error!\n");
        fprintf(2, "pid: %d\n", getpid());
        fprintf(2, "n: %d\n", n);
        fprintf(2, "prime: %d\n", prime);
        exit(1);
    }

    fprintf(1, "prime %d\n", prime);
    
    int new_pipe[2];
    pipe(new_pipe);
    int pid = fork();

    if (pid == 0)
    {
        is_primes(new_pipe);
    }
    else
    {
        // 关闭右边管道读端
        close(new_pipe[0]);
        int temp = 0;
        while (read(p[0], &temp, 4))
        {
            if (temp % prime) write(new_pipe[1], &temp, 4);
        }
        // 关闭当前右边管道写端
        close(new_pipe[1]);
        // 关闭左边管道读端
        close(p[0]);
        wait((int *) 0);
    }
    exit(0);
}

int
main(int argc, char *argv[])
{
    int p[2];
    pipe(p);

    int pid = fork();
    if (pid == 0)
    {
        is_primes(p);
    }
    else
    {
        close(p[0]);
        int prime = 2;
        for (int i = 2; i <= 35; i++)
        {
            if (i == 2)
            {
                fprintf(1, "prime %d\n", i);
                continue;
            }

            if (i % prime) 
            {
                write(p[1], &i, 4);
            }
        }
        close(p[1]);
        wait((int *) 0);
        exit(0);
    }
    return 0;
}