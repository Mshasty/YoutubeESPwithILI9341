#include <time.h>
#include <TimeLib.h>
#include <NtpClientLib.h>

#define NTP_PAUSE_FIRST 9000
#define NTP_PAUSE_CONTINUE 1500000
unsigned long ntp_timer = NTP_PAUSE_FIRST; 
unsigned long ntp_lasttime;   //last time print has been done

bool get_ip;
bool time_start = false;

void processSyncEvent(NTPSyncEvent_t ntpEvent) {
    if (ntpEvent) {
        DEBG("Ошибка NTP: ");
        if (ntpEvent == noResponse) {
            DEBG("Сервер не ответил");
            ntp_timer = NTP_PAUSE_FIRST;
        }
        else if (ntpEvent == invalidAddress)
            DEBG("Не верный адрес NTP сервера");
    } else {
        DEBG("Получено время NTP: ");
        DEBG(NTP.getTimeDateString(NTP.getLastNTPSync()));
        ntp_timer = NTP_PAUSE_CONTINUE;
        time_start = false;
    }
}

void ntp(int interval, String NTPhost, int8_t timeZone) {
    static bool ntp_setup;
    if (!get_ip) return;
    if (!ntp_setup) {
        NTP.begin(NTPhost, timeZone, true, 0);
        NTP.setInterval(interval);
        ntp_setup = true;
    }
    DEBG(NTP.getTimeDateString() + " Включено: " + NTP.getUptimeString() + "  Старт: " + NTP.getTimeDateString(NTP.getFirstSync()).c_str());
    time_start = false;
    if (year() > 2019)
        ntp_timer = NTP_PAUSE_CONTINUE;

}

String convertMon(int n) {
  switch (n) {
    case 1: return F("Jan");
    case 2: return F("Feb");
    case 3: return F("Mar");
    case 4: return F("Apr");
    case 5: return F("May");
    case 6: return F("Jun");
    case 7: return F("Jul");
    case 8: return F("Aug");
    case 9: return F("Sen");
    case 10: return F("Okt");
    case 11: return F("Nov");
    case 12: return F("Dec");
    default: return F("???"); 
  }
}

String convertWeek(int n) {
  switch (n) {
    case 2: return F("Monday");
    case 3: return F("Tuesday");
    case 4: return F("Wednesday");
    case 5: return F("Thursday");
    case 6: return F("Friday");
    case 7: return F("Saturday");
    case 1: return F("Sunday");
    default: return F("My_day");
  }
}

String timeString() {
    //if (hour()+minute()+second() == 0) time_start = false;
    return String(hour() / 10) + String(hour() % 10) + " : " + String(minute() / 10) + String(minute() % 10)  + " : " + String(second() / 10) + String(second() % 10);
}

String dateString() {
    return convertWeek(weekday()) + " " + String(day()) + " " + convertMon(month()) + " " + String(year());
}

String ipToString(IPAddress ip)
{
    String s = "";
    for (int i = 0; i < 4; i++)
        s += i ? "." + String(ip[i]) : String(ip[i]);
    return s;
} 