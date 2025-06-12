#include <wiringPi.h>
#include <stdio.h>
#include <unistd.h>

int main() {
    wiringPiSetupGpio();
    pinMode(17, INPUT);
    pullUpDnControl(17, PUD_DOWN); // Important!

    while (1) {
        int value = digitalRead(17);
        printf("GPIO17: %d\n", value);
        usleep(200000);
    }

    return 0;
}
