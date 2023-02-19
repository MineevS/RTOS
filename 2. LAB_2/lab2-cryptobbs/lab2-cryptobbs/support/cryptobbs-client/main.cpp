#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <iostream>

int main( int argc, char **argv ) 
{
    // open a connection to the server (fd == coid)
    int fd = open("/dev/cryptobbs", O_RDWR);
    if(fd < 0)
    {
        std::cerr << "E: unable to open server connection: " << strerror(errno ) << std::endl;
        return EXIT_FAILURE;
    }

    close(fd);

    return EXIT_SUCCESS;
}
