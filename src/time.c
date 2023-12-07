#include "sys.h"

#define CURRENT_YEAR 2023

static unsigned char rtc_read(int reg)
{
    outb(0x70, reg);
    return inb(0x71);
}

static int get_update_in_progress_flag()
{
    outb(0x70, 0x0A);
    return (inb(0x71) & 0x80);
}

void datetime(datetime_t* dest)
{
    int century_register = 0x00; 

    unsigned char second;
    unsigned char minute;
    unsigned char hour;
    unsigned char day;
    unsigned char month;
    unsigned int year;
    unsigned char century;

    unsigned char last_second;
    unsigned char last_minute;
    unsigned char last_hour;
    unsigned char last_day;
    unsigned char last_month;
    unsigned char last_year;
    unsigned char last_century;
    unsigned char registerB;

    // Note: This uses the "read registers until you get the same values twice in a row" technique
    //       to avoid getting dodgy/inconsistent values due to RTC updates

    while (get_update_in_progress_flag());                // Make sure an update isn't in progress
    second = rtc_read(0x00);
    minute = rtc_read(0x02);
    hour = rtc_read(0x04);
    day = rtc_read(0x07);
    month = rtc_read(0x08);
    year = rtc_read(0x09);
    if (century_register != 0) {
        century = rtc_read(century_register);
    }

    do {
        last_second = second;
        last_minute = minute;
        last_hour = hour;
        last_day = day;
        last_month = month;
        last_year = year;
        last_century = century;

        while (get_update_in_progress_flag());           // Make sure an update isn't in progress
        second = rtc_read(0x00);
        minute = rtc_read(0x02);
        hour = rtc_read(0x04);
        day = rtc_read(0x07);
        month = rtc_read(0x08);
        year = rtc_read(0x09);
        if(century_register != 0) {
                century = rtc_read(century_register);
        }
    } while( (last_second != second) || (last_minute != minute) || (last_hour != hour) ||
            (last_day != day) || (last_month != month) || (last_year != year) ||
            (last_century != century) );

    registerB = rtc_read(0x0B);

    // Convert BCD to binary values if necessary

    if (!(registerB & 0x04)) {
        second = (second & 0x0F) + ((second / 16) * 10);
        minute = (minute & 0x0F) + ((minute / 16) * 10);
        hour = ( (hour & 0x0F) + (((hour & 0x70) / 16) * 10) ) | (hour & 0x80);
        day = (day & 0x0F) + ((day / 16) * 10);
        month = (month & 0x0F) + ((month / 16) * 10);
        year = (year & 0x0F) + ((year / 16) * 10);
        if(century_register != 0) {
                century = (century & 0x0F) + ((century / 16) * 10);
        }
    }

    // Convert 12 hour clock to 24 hour clock if necessary

    if (!(registerB & 0x02) && (hour & 0x80)) {
        hour = ((hour & 0x7F) + 12) % 24;
    }

    // Calculate the full (4-digit) year

    if(century_register != 0) {
        year += century * 100;
    } else {
        year += (CURRENT_YEAR / 100) * 100;
        if(year < CURRENT_YEAR) year += 100;
    }

    dest->second = second;
    dest->minute = minute;
    dest->hour = hour;
    dest->day = day;
    dest->month = month;
    dest->year = year;
}

unsigned long time()
{
    datetime_t dt;
    datetime(&dt);
    return (unsigned long) (((dt.year - 1970) * 3.154e7) + (dt.month * 2.628e+6) + (dt.day * 86400.0) + (dt.hour * 3600.0) + (dt.minute * 60.0) + dt.second);
}