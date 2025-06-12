#ifndef SSD1306_H
#define SSD1306_H

#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstring>
#include <cstdint>
#include <cstdio>

#define SSD1306_ADDR 0x3C
#define WIDTH 128
#define HEIGHT 64

class SSD1306 {
    int fd;
    uint8_t buffer[WIDTH * HEIGHT / 8];

public:
    SSD1306() {
        memset(buffer, 0, sizeof(buffer));
    }

    bool begin(const char* i2cPath = "/dev/i2c-1") {
        fd = open(i2cPath, O_RDWR);
        if (fd < 0) {
            perror("Failed to open I2C");
            return false;
        }
        if (ioctl(fd, I2C_SLAVE, SSD1306_ADDR) < 0) {
            perror("Failed to connect to display");
            return false;
        }

        static const uint8_t init1[] = {
            0xAE, 0xD5, 0x80, 0xA8, 0x3F, 0xD3, 0x00,
            0x40, 0x8D, 0x14, 0x20, 0x00, 0xA1, 0xC8,
            0xDA, 0x12, 0x81, 0xCF, 0xD9, 0xF1, 0xDB,
            0x40, 0xA4, 0xA6, 0xAF
        };
        sendCommand(init1, sizeof(init1));
        return true;
    }

    void sendCommand(const uint8_t* cmd, size_t len) {
        uint8_t data[len + 1];
        data[0] = 0x00;
        memcpy(data + 1, cmd, len);
        write(fd, data, len + 1);
    }

    void sendBuffer() {
        for (int page = 0; page < 8; ++page) {
            uint8_t cmds[] = { 0xB0 + page, 0x00, 0x10 };
            sendCommand(cmds, sizeof(cmds));
            uint8_t data[129];
            data[0] = 0x40;
            memcpy(data + 1, buffer + WIDTH * page, 128);
            write(fd, data, 129);
        }
    }

    void clear() {
        memset(buffer, 0, sizeof(buffer));
    }

    void drawPixel(int x, int y) {
        if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return;
        buffer[x + (y / 8) * WIDTH] |= (1 << (y % 8));
    }

    void drawChar(int x, int y, char c);
    void drawText(int x, int y, const char* str);
};

#endif
