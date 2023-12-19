#include <stdint.h>

#include "../multiboot.h"
#include "../stdlib/util.h"
#include "../mm/kheap.h"
#include "../fs/fs.h"
#include "../drivers/tty.h"
#include "../drivers/cmos.h"
#include "../mm/mmap.h"
#include "../mm/pmm.h"

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

void
shell_execute(char** args, multiboot_info_t* mbi)
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
            uint8_t contents[1024] = {0};
            uint32_t fd = open_file(args[1], FILE_OVERWRITE_FLAG);
            read_file(fd, contents, get_file_size(fd));
            kprintf("%s\n", contents);
        }
    } else if (!kstrcmp(args[0], "write")) {
        if (args[1] != NULL) {
            uint8_t flags = 0;
            if (!kstrcmp(args[1], "-a")) {
                flags = FILE_APPEND_FLAG;
            } else if (!kstrcmp(args[1], "-o")) {
                flags = FILE_OVERWRITE_FLAG;
            }
            read_fs_header();
            uint32_t fd = open_file(args[3], flags);
            write_file(fd, (uint8_t*)args[2], strlen(args[2]));
            write_fs_header();
        }
    } else if (!kstrcmp(args[0], "credits")) {
        kprintf("\tParOS\n\tBy: ColexDev\n");
    } else if (!kstrcmp(args[0], "ping")) {
        kprintf("pong!\n");
    } else if (!kstrcmp(args[0], "clear")) {
        clear_screen();
    } else if (!kstrcmp(args[0], "time")) {
        char* time;
        kprintf("Time: 0x%x\n", &time);
        get_time_string(time);
        kprintf("%s\n", time);
    } else if (!kstrcmp(args[0], "date")) {
        char* date;
        kprintf("Date: 0x%x\n", &date);
        get_date_string(date);
        kprintf("%s\n", date);
    } else if (!kstrcmp(args[0], "memmap")) {
        parse_multiboot_mmap(mbi);
    } else if (!kstrcmp(args[0], "memused")) {
        kprintf("%d bytes\n", pmm_get_used_memory());
    } else if (!kstrcmp(args[0], "memreserved")) {
        char buf[32] = {0};
        itoa(pmm_get_reserved_memory(), buf, 10);
        puts(buf);
        puts(" bytes\n");
    } else if (!kstrcmp(args[0], "ls")) {
        read_fs_header();
        list_files();
    } else if (!kstrcmp(args[0], "exit")) {
    } else {
        kprintf("Error: Command not found\n");
    }
}

void
shell_loop(multiboot_info_t* mbi)
{
    char* line;
    char** args;
    uint32_t addr;

    /* FIXME: Somehow 8+ characters in "line" causes a memory leak, it seems fine until you enter
     * another command, in which it does way more allocs than normal and creates a leak? */
    for (;;) {
        kprintf("> ");
        line = kgets();
        args = shell_parse_line(line);
        shell_execute(args, mbi);

        memset(line, 0, sizeof(char) * 128);
        memset(args, 0, sizeof(char*) * 64);
        // kprintf("FREEING LINE\n");
        kfree(line);
        kfree(args);
    }
}
