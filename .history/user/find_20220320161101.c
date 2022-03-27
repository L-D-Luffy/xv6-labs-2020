/*
    难点：
        * 如何去获取对应目录下的路径
        * 如何去递归处理每一个文件夹
        * 如何去如何去维护好递归前后的状态
    想法：
        * 感觉有点像是一道DFS的题，笑死
    一些问题
        * 话说目录当成文件打开后得到的东西是啥？看样子可以读到dirent结构体里面去就是了

*/

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"


void
find(char *path, char *des)
{
    // 先打开当前path下所有的文件和目录
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;
    // 先打开当前目录
    if ((fd = open(path, 0)) < 0)
    {
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }
    // 然后判断打开的类型，如果是文件应该是返回错误吧
    if(fstat(fd, &st) < 0)
    {
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }
}

int
main(int argc, char *argv[])
{
    int i;
    if (argc < 3)
    {
        fprintf(2, "usage: find [dir] [des]\n");
    }

    find(argv[1], argv[2]);

    exit(0);
}