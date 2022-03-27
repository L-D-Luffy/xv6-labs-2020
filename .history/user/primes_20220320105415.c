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

*/

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
    int p[2];
    pipe(p);

    int pid = fork();
    if (pid == 0)
    {
        close(p[1]);
        is_primes(p);
    }
}