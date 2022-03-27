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
        int n = read(p[0], buf, sizeof(buf));
        if (n == 1) 
        {
            fprintf(1, "%d: received ping", getpid());
        }
        write(p[1], '1', 1);

        exit(0);
    }
    else if ()
    {
        write(p[1], '1', 1);
        wait((int *) 0);
        int n = read(p[0], buf, sizeof(buf));
        if (n == 1)
        {
            fprintf(1, "%d: received pong", getpid());
        }
    }
}