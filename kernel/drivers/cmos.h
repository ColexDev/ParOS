#ifndef CMOS_H
#define CMOS_H

extern unsigned char second;
extern unsigned char minute;
extern unsigned char hour;
extern unsigned char day;
extern unsigned char month;
extern unsigned int  year;

void read_rtc();
void get_date_string(char* date_str);
void get_time_string(char* time_str);

#endif /* #ifndef CMOS_H */
