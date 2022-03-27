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

char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  // 如果刚好等于大小就直接返回，不然就填充
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  // 将后面的字符串填充为' '
  memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
  return buf;
}

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
    if (fstat(fd, &st) < 0)
    {
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    if (st.type != T_DIR)
    {
        fprintf(2, "usage: find [dir] [des]\n");
        return;
    }
    if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf)
    {
        fprintf(2, "find: path too long \n");
        return;
    }
    strcpy(buf, path);
    p = buf + strlen(buf);
    *p++ = '/';
    while (read(fd, &de, sizeof(de)) == sizeof(de))
    {
        //fprintf(1, "de.name: %s\n", de.name);
        if (de.inum == 0) continue;
        if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0) continue;
        // 拼接成目录，stat查看类型
        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = 0;
        //fprintf(1, "buf: %s\n", buf);
        if (stat(buf, &st) < 0)
        {
            fprintf(2, "find: cannot stat %s\n", buf);
            continue;
        }
        if (st.type == T_FILE)
        {
            
            if (strcmp(de.name, des) == 0)
            {
                fprintf(1, "%s\n", buf);
            }
        }
        else if (st.type == T_DIR)
        {
            find(buf, des);
        }
    }
    close(fd);
}

int
main(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(2, "usage: find [dir] [des]\n");
    }

    find(argv[1], argv[2]);

    exit(0);
}