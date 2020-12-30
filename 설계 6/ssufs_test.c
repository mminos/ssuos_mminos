#include "ssufs-ops.h"

int main()
{
    char str[] = "!-------32 Bytes of Data-------!!-------32 Bytes of Data-------!";
    char str32[] = "!-------32 Bytes of Data-------!";
    ssufs_formatDisk();

    ssufs_create("f1.txt");
    int fd1 = ssufs_open("f1.txt");

    ssufs_create("f2.txt");
    int fd2 = ssufs_open("f2.txt");

    printf("Write Data: %d\n", ssufs_write(fd1, str, BLOCKSIZE));
    printf("Seek: %d\n", ssufs_lseek(fd1, 0));
    ssufs_dump();

    printf("Write Data: %d\n", ssufs_write(fd2, str, BLOCKSIZE));
    printf("Seek: %d\n", ssufs_lseek(fd2, 0));
    ssufs_dump();

    printf("Write Data: %d\n", ssufs_write(fd2, str32, 32));
    printf("Seek: %d\n", ssufs_lseek(fd2, 0));
    ssufs_dump();

    ssufs_write(fd2, ".", 1);
    ssufs_dump();

    printf("Write Data: %d\n", ssufs_write(fd2, str32, 32));
    printf("Seek: %d\n", ssufs_lseek(fd2, 0));
    ssufs_dump();

    printf("Write Data: %d\n", ssufs_write(fd2, str32, 32));
    printf("Seek: %d\n", ssufs_lseek(fd2, 0));
    ssufs_dump();

    printf("Write Data: %d\n", ssufs_write(fd1, str, BLOCKSIZE));
    printf("Seek: %d\n", ssufs_lseek(fd1, 0));
    ssufs_dump();

    printf("Write Data: %d\n", ssufs_write(fd1, str32, 32));
    printf("Seek: %d\n", ssufs_lseek(fd1, 0));
    ssufs_dump();

    ssufs_delete("f1.txt");
    ssufs_dump();
    ssufs_delete("f2.txt");
    ssufs_dump();
}
