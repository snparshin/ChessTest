#include <iostream>
#include <vector>
#include <string>
#include <queue>

using namespace std;

// структура для хранения текущей клетки и расстояния до неё
struct Cell {
    int x, y;
    int dist;
};

// проверка на выход из за пределы доскит
bool isValid(int x, int y) {
    return (x >= 0 && x < 8 && y >= 0 && y < 8);
}

// функция knight
int knight(std::string pos1, std::string pos2) {
    //Конвертируем шахматные координаты (a1..h8) в индексы массива (0..7)
    int startX = pos1[0] - 'a';
    int startY = pos1[1] - '1';
    
    int endX = pos2[0] - 'a';
    int endY = pos2[1] - '1';

    if (!isValid(startX, startY) || !isValid(endX, endY)) return -1;
    
    // Если стоим на месте - 0 ходов
    if (startX == endX && startY == endY) return 0;

    // Вариант того, как может ходить конь
    int dx[] = { -2, -1, 1, 2, 2, 1, -1, -2 };
    int dy[] = { 1, 2, 2, 1, -1, -2, -2, -1 };

    // Поиск в ширину
    queue<Cell> q;
    q.push({startX, startY, 0});

    // Массив посещенных клеток
    bool visited[8][8];
    
    for(int i = 0; i < 8; i++) 
        for(int j = 0; j < 8; j++) 
            visited[i][j] = false;

    visited[startX][startY] = true;

    while (!q.empty()) {
        Cell current = q.front();
        q.pop();

        
        if (current.x == endX && current.y == endY) {
            return current.dist;
        }

        // Проверяем все возможные ходы коня из текущей точки
        for (int i = 0; i < 8; i++) {
            int newX = current.x + dx[i];
            int newY = current.y + dy[i];

            
            if (isValid(newX, newY) && !visited[newX][newY]) {
                visited[newX][newY] = true;
                q.push({newX, newY, current.dist + 1});
            }
        }
    }

    return -1; // Путь не найден 
}

int main() {
    string p1, p2;
    
    cout << "--- Knight Moves Calculator ---" << endl;
    cout << "Format: a1 h8 (type 'exit' to quit)" << endl;

    // Цикл запроса данных
    while (true) {
        cout << "\nEnter start pos (pos1): ";
        cin >> p1;
        if (p1 == "exit") break;

        cout << "Enter end pos (pos2): ";
        cin >> p2;
        if (p2 == "exit") break;

        int result = knight(p1, p2);
        
        if (result != -1) {
            cout << "Min moves: " << result << endl;
        } else {
            cout << "Invalid input!" << endl;
        }
    }
    return 0;
}