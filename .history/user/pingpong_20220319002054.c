#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

char buf[10];

int
main(int argc, char *argv[])
{
    int p[2];
    pipe(p);
    if (fork() == 0)
    {
        read(p[0], buf, );
    }
}