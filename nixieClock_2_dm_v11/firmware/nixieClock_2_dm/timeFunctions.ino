// отправить на индикаторы
void sendTime(byte hours, byte minutes) {
  indiDigits[0] = hours / 10;
  indiDigits[1] = hours % 10;
  indiDigits[2] = minutes / 10;
  indiDigits[3] = minutes % 10;
}

// для эффектов
void setNewTime() {
    int8_t newHrs = hrs, newMins = mins, newSecs;
    newSecs = secs + 1; 
    if (newSecs >= 60) { newSecs = 0; newMins++; if (newMins >= 60) { newMins = 0; newHrs++; if (newHrs >= 24) newHrs = 0; } }
    newTime[0] = (byte)newHrs / 10;
    newTime[1] = (byte)newHrs % 10;
    newTime[2] = (byte)newMins / 10;
    newTime[3] = (byte)newMins % 10;
}

void syncRTC() {
    DateTime now = rtc.now();
    secs = now.second();
    mins = now.minute();
    hrs = now.hour();
}

int getNumberOfDays(uint16_t year, byte month) {
    if (month == 2) {
        if ((year % 400 == 0) || (year % 4 == 0 && year % 100 != 0)) return 29;
        else return 28;
    }
    else if (month == 1 || month == 3 || month == 5 || month == 7 || month == 8 || month == 10 || month == 12) return 31;
    else return 30;
}
