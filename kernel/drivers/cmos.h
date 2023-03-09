#ifndef CMOS_H
#define CMOS_H

extern unsigned char second;
extern unsigned char minute;
extern unsigned char hour;
extern unsigned char day;
extern unsigned char month;
extern unsigned int  year;

void read_rtc();
void print_date();
void print_time();

#endif /* #ifndef CMOS_H */
