/*
    问题：
        * 命令行里面是怎么处理管道类型的输入的？
            应该就是当做标准输入读进来的
        * read如何处理\n？

*/

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(2, "usage: xargs command ...");
    }

    
}