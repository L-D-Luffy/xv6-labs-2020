/*
    主要用到的系统调用
    * int read(int, void*, int);
    * int write(int, const void*, int);
    * int pipe(int*);
    * int fork(void);
    * int wait(int*);
    
    主要思路：
    1. 理解什么是质数筛 https://zhuanlan.zhihu.com/p/100051075
    2. 加速的关键在于，

*/

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
    
}