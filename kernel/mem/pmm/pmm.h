#ifndef PMM_H
#define PMM_H

#define PAGE_SIZE 4096
#define WORD_LENGTH 8

#define WORD_OFFSET(b) ((b) / WORD_LENGTH)
#define BIT_OFFSET(b)  ((b) % WORD_LENGTH)

void pmm_init(void);

#endif /* PMM_H */
