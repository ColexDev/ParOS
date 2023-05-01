#include <stdint.h>

#include "../multiboot.h"
#include "../stdlib/util.h"
#include "../mm/kheap.h"
#include "../fs/fs.h"
#include "../drivers/tty.h"
#include "../drivers/cmos.h"

#define BUF_SIZE 64

char**
shell_parse_line(char* line)
{
    uint8_t position = 0;
    char* token = {0};
    char** tokens = kmalloc(BUF_SIZE * sizeof(char*));
    uint8_t i = 0;

    memset(tokens, 0, 256);
    memset(token, 0, 8);
    kstrcat(line, " ");
    token = kstrtok(line, " ");
    while (token != NULL) {
        tokens[i] = token;
        i++;
        token = kstrtok(NULL, " ");
    }
    tokens[i] = NULL;
    return tokens;
}

uint8_t
shell_execute(char** args)
{
    if (!kstrcmp(args[0], "touch")) {
        if (args[1] != NULL) {
            // args[1][strlen(args[1])] = '\0';
            read_fs_header();
            create_file(args[1]);
            write_fs_header();
        }
    } else if (!kstrcmp(args[0], "rm")) {
        if (args[1] != NULL) {
            read_fs_header();
            delete_file(args[1]);
            write_fs_header();
        }
    } else if (!kstrcmp(args[0], "cat")) {
        if (args[1] != NULL) {
            read_fs_header();
            uint8_t contents[512] = {0};
            uint32_t fd = open_file(args[1]);
            read_file(fd, contents, get_file_size(fd));
            kprintf("%s\n", contents);
        }
    } else if (!kstrcmp(args[0], "write")) {
        if (args[1] != NULL) {
            read_fs_header();
            uint32_t fd = open_file(args[2]);
            write_file(fd, (uint8_t*)args[1], strlen(args[1]));
            write_fs_header();
        }
    } else if (!kstrcmp(args[0], "credits")) {
        kprintf("\tParOS\n\tBy: ColexDev\n");
    } else if (!kstrcmp(args[0], "ping")) {
        kprintf("pong!\n");
    // } else if (!kstrcmp(args[0], "panic")) {
    //     kprintf("Okay... You asked for it...\nPANIC\n");
    //     kernel_panic();
    } else if (!kstrcmp(args[0], "clear")) {
        clear_screen();
    } else if (!kstrcmp(args[0], "time")) {
        char* time;
        get_time_string(time);
        kprintf("%s\n", time);
    } else if (!kstrcmp(args[0], "date")) {
        char* date;
        get_date_string(date);
        kprintf("%s\n", date);
    } else if (!kstrcmp(args[0], "memmap")) {
        // parse_multiboot_mmap(mbi);
    } else if (!kstrcmp(args[0], "memused")) {
        char buf[32] = {0};
        // itoa(pmm_get_used_memory(), buf, 10);
        // puts(buf);
        // puts(" bytes\n");
    } else if (!kstrcmp(args[0], "memreserved")) {
        char buf[32] = {0};
        // itoa(pmm_get_reserved_memory(), buf, 10);
        // puts(buf);
        // puts(" bytes\n");
    } else if (!kstrcmp(args[0], "ls")) {
        list_files();
    } else if (!kstrcmp(args[0], "exit")) {
    } else {
        kprintf("Error: Command not found\n");
    }
    return 1;
}

void
shell_loop(multiboot_info_t* mbi)
{
    char* line;
    char** args;
    uint8_t status = 1;
    uint32_t addr;

    do {
        kprintf("> ");
        line = kgets();
        args = shell_parse_line(line);
        status = shell_execute(args);

        memset(line, 0, sizeof(char*));
        kfree(line);
        kfree(args);
    } while (status);
}
