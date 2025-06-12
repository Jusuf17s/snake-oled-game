#include "ssd1306.h"
#include <unistd.h>

int main() {
    SSD1306 display;

    if (!display.begin()) {
        return 1; // Error connecting to OLED
    }

    display.clear();
    display.drawText(10, 30, "HELLO YUSUF");
    display.sendBuffer();

    // Keep message on screen
    while (1) {
        usleep(100000);
    }

    return 0;
}
