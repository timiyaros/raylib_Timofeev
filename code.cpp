#include "raylib.h"
#include <math.h>
#include <stdlib.h>

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define PLAYER_SPEED 5.0f
#define BULLET_SPEED 10.0f
#define TARGET_SPEED 2.0f
#define MAX_ENEMIES 50

typedef struct {
    Vector2 position;
    Vector2 velocity;
    bool active;
} Enemy;

Enemy enemies[MAX_ENEMIES];

Vector2 playerPos = { 910.0f, 510.0f };  // Позиция игрока
Vector2 bulletPos = { 0.0f, 0.0f };      // Позиция пули
Vector2 bulletVelocity = { 0.0f, 0.0f }; // Направление пули

bool bulletActive = false;  // Флаг для пули
int health = 3;             // ХП игрока
int shirHealth = 150;       // Ширина ХП
int count = 0;              // Счётчик убийств
bool game = true;           // Игра продолжается
int waveNumber = 0;         // Номер волны

// Нормализация вектора
Vector2 NormalizeVector(Vector2 vec) {
    float length = sqrtf(vec.x * vec.x + vec.y * vec.y);
    if (length > 0) {
        vec.x /= length;
        vec.y /= length;
    }
    return vec;
}

// Функция для проверки, уничтожены ли все враги
bool AreAllEnemiesDead() {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            return false;  // Если хоть один враг активен, возвращаем false
        }
    }
    return true;  // Все враги мертвы
}

// Функция для инициализации волны врагов
void SpawnWave(int numEnemies) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (i < numEnemies) {
            enemies[i].active = true;

            // Рандомный спавн за пределами экрана
            int side = GetRandomValue(1, 4);
            if (side == 1) {  // Слева
                enemies[i].position.x = -10;
                enemies[i].position.y = GetRandomValue(0, SCREEN_HEIGHT);
            }
            if (side == 2) {  // Сверху
                enemies[i].position.x = GetRandomValue(0, SCREEN_WIDTH);
                enemies[i].position.y = -10;
            }
            if (side == 3) {  // Справа
                enemies[i].position.x = SCREEN_WIDTH + 10;
                enemies[i].position.y = GetRandomValue(0, SCREEN_HEIGHT);
            }
            if (side == 4) {  // Снизу
                enemies[i].position.x = GetRandomValue(0, SCREEN_WIDTH);
                enemies[i].position.y = SCREEN_HEIGHT + 10;
            }
        }
        else {
            enemies[i].active = false;  // Отключаем лишних врагов
        }
    }
}

// Получение ближайшего врага
Vector2 GetNearestEnemy() {
    float minDistance = 1e6;
    Vector2 nearestPos = { 0, 0 };

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) {
            float dist = sqrtf((enemies[i].position.x - playerPos.x) * (enemies[i].position.x - playerPos.x) +
                (enemies[i].position.y - playerPos.y) * (enemies[i].position.y - playerPos.y));
            if (dist < minDistance) {
                minDistance = dist;
                nearestPos = enemies[i].position;
            }
        }
    }
    return nearestPos;
}

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Enemy Waves Game");
    SetTargetFPS(60);

    waveNumber = 1;
    SpawnWave(5); // Начальная волна

    while (!WindowShouldClose() && game) {
        // Спавн новой волны, если все враги мертвы
        if (AreAllEnemiesDead()) {
            waveNumber++;
            SpawnWave(5 + waveNumber * 2);  // Увеличение количества врагов на каждой волне
        }

        // Движение игрока
        if (IsKeyDown(KEY_W) && playerPos.y > 0) playerPos.y -= PLAYER_SPEED;
        if (IsKeyDown(KEY_S) && playerPos.y < SCREEN_HEIGHT - 50) playerPos.y += PLAYER_SPEED;
        if (IsKeyDown(KEY_A) && playerPos.x > 0) playerPos.x -= PLAYER_SPEED;
        if (IsKeyDown(KEY_D) && playerPos.x < SCREEN_WIDTH - 50) playerPos.x += PLAYER_SPEED;

        // Стрельба
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !bulletActive) {
            bulletPos = playerPos;
            bulletActive = true;
            Vector2 nearestEnemy = GetNearestEnemy();
            bulletVelocity = NormalizeVector(Vector2{ nearestEnemy.x - bulletPos.x, nearestEnemy.y - bulletPos.y });
        }

        // Движение пули
        if (bulletActive) {
            bulletPos.x += bulletVelocity.x * BULLET_SPEED;
            bulletPos.y += bulletVelocity.y * BULLET_SPEED;

            if (bulletPos.x < 0 || bulletPos.x > SCREEN_WIDTH || bulletPos.y < 0 || bulletPos.y > SCREEN_HEIGHT)
                bulletActive = false;

            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (enemies[i].active &&
                    CheckCollisionRecs(Rectangle{ bulletPos.x, bulletPos.y, 10, 10 },
                        Rectangle{
                    enemies[i].position.x, enemies[i].position.y, 30, 30
                        })) {
                    enemies[i].active = false;
                    bulletActive = false;
                    count++;
                }
            }
        }

        // Движение врагов к игроку
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (enemies[i].active) {
                Vector2 direction = NormalizeVector(Vector2{ playerPos.x - enemies[i].position.x, playerPos.y - enemies[i].position.y });
                enemies[i].position.x += direction.x * TARGET_SPEED;
                enemies[i].position.y += direction.y * TARGET_SPEED;

                // Столкновение с игроком
                if (CheckCollisionRecs(Rectangle{ playerPos.x, playerPos.y, 50, 50 },
                    Rectangle{
                    enemies[i].position.x, enemies[i].position.y, 30, 30
                    })) {
                    enemies[i].active = false;
                    health--;
                    shirHealth -= 50;
                    if (health <= 0) game = false;
                }
            }
        }

        // Отрисовка
        BeginDrawing();
        ClearBackground(BLACK);

        DrawRectangleV(playerPos, Vector2{ 50, 50 }, BLUE);  // Игрок
        if (bulletActive) DrawRectangleV(bulletPos, Vector2{ 10, 10 }, YELLOW);  // Пуля

        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (enemies[i].active) DrawRectangleV(enemies[i].position, Vector2{ 30, 30 }, GREEN);
        }

        DrawRectangle(10, 10, shirHealth, 20, RED);  // Полоска ХП
        DrawText(TextFormat("Kills: %i", count), 10, 40, 20, WHITE);
        DrawText(TextFormat("Health: %i", health), 10, 70, 20, WHITE);
        DrawText(TextFormat("Wave: %i", waveNumber), 10, 100, 20, WHITE);

        if (!game) DrawText("GAME OVER", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2, 40, RED);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
