/*
 * This is a helper file for handling asynchronous delays in seconds
 * and minutes
 */

void AsyncDelay_StartTimer(unsigned long *timer) {
  *timer = millis();
}

bool AsyncDelay_HasMillisElapsed(unsigned long millisTimer, unsigned int millisInterval) {
  return (millis() - millisTimer) >= millisInterval;
}

bool AsyncDelay_HasSecondsElapsed(unsigned long secondsTimer, unsigned int secondsInterval) {
  unsigned long secondsMillis = AsyncDelay_MillisFromSeconds(secondsInterval);
  return (millis() - secondsTimer) >= secondsMillis;
}

unsigned long AsyncDelay_RemainingSeconds(unsigned long secondsTimer, unsigned int secondsInterval) {
  return (secondsInterval - ((millis() - secondsTimer) / 1000));
}

bool AsyncDelay_HasMinutesElapsed(unsigned long minutesTimer, unsigned int minutesInterval) {
  unsigned long minutesMillis = AsyncDelay_MillisFromMinutes(minutesInterval);
  return (millis() - minutesTimer) >= minutesMillis;
}

unsigned long AsyncDelay_RemainingMinutes(unsigned long minutesTimer, unsigned int minutesInterval) {
  return (minutesInterval - ((millis() - minutesTimer) / 1000 / 60));
}

unsigned long AsyncDelay_MillisFromSeconds(unsigned int seconds) {
  return seconds * 1000;
}

unsigned long AsyncDelay_MillisFromMinutes(unsigned int minutes) {
  return minutes * 1000 * 60;
}
