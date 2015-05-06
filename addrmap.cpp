#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

int main(int argc, char** argv)
{
    const off_t slave_base = 0xff200000;
    const off_t mem_if_base = 0x10010;
    int map_length = mem_if_base + sizeof(uint64_t);

    uint32_t addr = 0x20000000;
    if (argc > 1) {
        char* end;
        addr = strtol(argv[1], &end, 0);
    }
    printf("Physical Addr: 0x%x\n", addr);

    int fd = open("/dev/mem", O_RDWR | O_SYNC); // uncache
    if (fd < 0)
        exit(1);
    //shm_ctl(fd, SHMCTL_PHYS, salve_base, 0, map_length);
    volatile uint8_t* mapped_base = reinterpret_cast< uint8_t* >(mmap(NULL, map_length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, slave_base));
    if (mapped_base == MAP_FAILED)
        close(fd);
    printf("mmap: cmd vaddr:%p\n", mapped_base);

    volatile uint32_t* data_base = reinterpret_cast< volatile uint32_t* >(mmap(NULL, 0x100000, PROT_READ, MAP_SHARED, fd, addr));
    if (data_base == MAP_FAILED)
        close(fd);
    printf("mmap: data vaddr:%p\n", data_base);

    volatile uint32_t* cmd = reinterpret_cast< volatile uint32_t* >(mapped_base + mem_if_base);
    printf("Command: 0x%x\n", *(cmd));
    *cmd = 0xff;
    printf("Command: 0x%x (issued)\n", *(cmd));
    usleep(3000000);
    printf("Command: 0x%x (waited)\n", *(cmd));
    *cmd = 0;
    printf("Command: 0x%x (stopped)\n", *(cmd));

    for (int i = 0; i < 32;) {
        for (int j = 0; j < 4; j ++, i ++) {
            printf("(%d) %08x ", i, *(data_base + i));
        }
        printf("\n");
    }

    printf("....\n");

    volatile uint32_t* last = data_base + ((0x2000 - 2) << 3); /* last 256bit */
    for (int i = 0; i < 32;) {
        for (int j = 0; j < 4; j ++, i ++) {
            printf("(%d) %08x ", i + (0x1ffc << 3) - 16, *(last + i)); 
        }
        printf("\n");
    }

    munmap((void*)data_base, 0x100000);
    munmap((void*)mapped_base, map_length);
    close(fd);
    return 0;
}
