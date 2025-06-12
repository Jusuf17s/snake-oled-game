#include "ssd1306.h"
#include <wiringPi.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <vector>
#include <string>
#include <mutex>
#include <iostream>

SSD1306 display;

#define BUZZER_PIN 18

struct Point {
    int x, y;
};

std::vector<Point> snake;
Point food;
char dir = 'R';
bool running = true;
bool restartRequested = false;
std::mutex dirMutex;
int score = 0;
time_t startTime;

void buzz(int durationMs) {
    digitalWrite(BUZZER_PIN, HIGH);
    usleep(durationMs * 1000);
    digitalWrite(BUZZER_PIN, LOW);
}

void drawBlock(int x, int y, int w, int h) {
    for (int dx = 0; dx < w; dx++) {
        for (int dy = 0; dy < h; dy++) {
            display.drawPixel(x + dx, y + dy);
        }
    }
}

void draw() {
    display.clear();
    for (auto &p : snake) {
        drawBlock(p.x, p.y, 4, 4);
    }
    drawBlock(food.x, food.y, 4, 4);

    std::string scoreText = "S:" + std::to_string(score);
    std::string timeText = "T:" + std::to_string((int)(time(NULL) - startTime)) + "s";
    display.drawText(0, 0, scoreText.c_str());
    display.drawText(80, 0, timeText.c_str());

    display.sendBuffer();
}

void setupButtons() {
    wiringPiSetupGpio();
    pinMode(27, INPUT); pullUpDnControl(27, PUD_UP); // UP
    pinMode(17, INPUT); pullUpDnControl(17, PUD_UP); // DOWN
    pinMode(22, INPUT); pullUpDnControl(22, PUD_UP); // RIGHT
    pinMode(23, INPUT); pullUpDnControl(23, PUD_UP); // LEFT
    pinMode(5, INPUT); pullUpDnControl(5, PUD_UP);   // RESTART BUTTON
    pinMode(6, INPUT); pullUpDnControl(6, PUD_UP);   // EXTRA RIGHT BUTTON
    pinMode(BUZZER_PIN, OUTPUT); digitalWrite(BUZZER_PIN, LOW); // Buzzer setup
}

void handleButtons() {
    while (running || !running) {
        dirMutex.lock();
        if (digitalRead(27) == LOW && dir != 'D') dir = 'U';
        else if (digitalRead(17) == LOW && dir != 'U') dir = 'D';
        else if ((digitalRead(22) == LOW || digitalRead(6) == LOW) && dir != 'L') dir = 'R';
        else if (digitalRead(23) == LOW && dir != 'R') dir = 'L';
        if (!running && (
            digitalRead(27) == LOW ||
            digitalRead(17) == LOW ||
            digitalRead(22) == LOW ||
            digitalRead(23) == LOW ||
            digitalRead(6) == LOW)) {
            restartRequested = true;
        }
        dirMutex.unlock();
        usleep(100000);
    }
}

void moveSnake() {
    while (running) {
        dirMutex.lock();
        Point head = snake.front();
        switch (dir) {
            case 'U': head.y -= 4; break;
            case 'D': head.y += 4; break;
            case 'L': head.x -= 4; break;
            case 'R': head.x += 4; break;
        }
        dirMutex.unlock();

        if (head.x < 0 || head.x >= 128 || head.y < 0 || head.y >= 64) {
            buzz(500);
            usleep(500000);
            running = false;
            break;
        }

        for (auto &p : snake) {
            if (head.x == p.x && head.y == p.y) {
                buzz(500);
                usleep(500000);
                running = false;
                return;
            }
        }

        snake.insert(snake.begin(), head);

        if (head.x == food.x && head.y == food.y) {
            food.x = (rand() % 30) * 4;
            food.y = (rand() % 16) * 4;
            score++;
            buzz(100);
        } else {
            snake.pop_back();
        }

        usleep(400000);
    }
}

void timerThread() {
    while (running) {
        usleep(1000000);
    }
}

void splashScreen() {
    for (int i = 3; i >= 1; i--) {
        display.clear();
        display.drawText(10, 10, "YUSUF'S");
        display.drawText(10, 25, "SLYTHERIN GAME");
        std::string msg = "STARTING IN " + std::to_string(i);
        display.drawText(10, 45, msg.c_str());
        display.sendBuffer();
        usleep(1000000);
    }
}

void resetGame() {
    snake.clear();
    snake.push_back({20, 20});
    food.x = (rand() % 30) * 4;
    food.y = (rand() % 16) * 4;
    dir = 'R';
    running = true;
    score = 0;
    startTime = time(NULL);
}

int main() {
    srand(time(0));
    display.begin();
    setupButtons();
    buzz(300);  // 🔊 Buzzer test on startup

    do {
        splashScreen();
        resetGame();
        restartRequested = false;

        std::thread t1(moveSnake);
        std::thread t2(handleButtons);
        std::thread t3(timerThread);

        while (running) {
            draw();
            usleep(50000);
        }

        t1.join();
        t2.detach();
        t3.join();

        display.clear();
        std::string finalScore = "SCORE: " + std::to_string(score);
        display.drawText(10, 20, "GAME OVER");
        display.drawText(10, 35, finalScore.c_str());
        display.drawText(10, 50, "PRESS RESTART");
        display.sendBuffer();

        while (!restartRequested) {
            usleep(100000);
        }
    } while (true);

    return 0;
}
