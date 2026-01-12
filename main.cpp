#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QMessageBox>
#include <QFontDatabase>
#include <queue>
#include <vector>
#include <algorithm>


struct Point {
    int x, y;
};

// Логика перемещения

// Проверка границ доски (0-7)
bool isValid(int x, int y) {
    return (x >= 0 && x < 8 && y >= 0 && y < 8);
}

// Алгоритм поиска кратчайшего пути коня
std::vector<Point> findPath(Point start, Point end) {
    if (!isValid(start.x, start.y) || !isValid(end.x, end.y)) return {};
    if (start.x == end.x && start.y == end.y) return {start};

    // Все возможные ходы коня
    int dx[] = { -2, -1, 1, 2, 2, 1, -1, -2 };
    int dy[] = { 1, 2, 2, 1, -1, -2, -2, -1 };

    Point parent[8][8];
    bool visited[8][8];

    // Инициализация массивов
    for(int i=0; i<8; i++) {
        for(int j=0; j<8; j++) {
            visited[i][j] = false;
            parent[i][j] = {-1, -1};
        }
    }

    // Запуск поиска в ширину
    std::queue<Point> q;
    q.push(start);
    visited[start.x][start.y] = true;

    bool found = false;

    while (!q.empty()) {
        Point curr = q.front();
        q.pop();

        // Если нашли цель
        if (curr.x == end.x && curr.y == end.y) {
            found = true;
            break;
        }

        // Перебор ходов
        for (int i = 0; i < 8; i++) {
            int nx = curr.x + dx[i];
            int ny = curr.y + dy[i];

            if (isValid(nx, ny) && !visited[nx][ny]) {
                visited[nx][ny] = true;
                parent[nx][ny] = curr;
                q.push({nx, ny});
            }
        }
    }

    if (!found) return {};

    // Восстановление пути от финиша к старту
    std::vector<Point> path;
    Point curr = end;
    while (curr.x != -1) {
        path.push_back(curr);
        if (curr.x == start.x && curr.y == start.y) break;
        curr = parent[curr.x][curr.y];
    }
    std::reverse(path.begin(), path.end());
    return path;
}

//GUI

class ChessBoard : public QWidget {
    Q_OBJECT

public:
    ChessBoard(QWidget *parent = nullptr) : QWidget(parent) {
        setFixedSize(460, 460); 
    }

    // Установка позиции коня и обновление экрана
    void setKnightPos(int x, int y) {
        knightX = x;
        knightY = y;
        update();
    }

protected:
    // Основной метод рисования
    void paintEvent(QPaintEvent *) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);      // Сглаживание фигур
        painter.setRenderHint(QPainter::TextAntialiasing);  // Сглаживание текста

        int margin = 30;     // Отступы для координат
        int boardSize = 400; // Размер самой доски
        int cellSize = boardSize / 8; // Размер клетки (50px)

        //Разметка поля
        painter.setPen(Qt::black);
        QFont font = painter.font();
        font.setBold(true);
        font.setPixelSize(14);
        painter.setFont(font);

        for (int i = 0; i < 8; ++i) {
            // Буквы снизу (a-h)
            QString letter = QChar('a' + i);
            int x = margin + i * cellSize;
            // Рисуем в прямоугольнике под доской
            painter.drawText(QRect(x, margin + boardSize, cellSize, 25), Qt::AlignCenter, letter);
            
            // Цифры слева (1-8)
            QString number = QString::number(i + 1);
            int y = margin + (7 - i) * cellSize;
            painter.drawText(QRect(0, y, margin - 5, cellSize), Qt::AlignRight | Qt::AlignVCenter, number);
        }

        //Отрисовка клеток доски
        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {
                int x = margin + i * cellSize;
                int y = margin + (7 - j) * cellSize; // Инверсия Y (снизу вверх)

                if ((i + j) % 2 == 0) painter.fillRect(x, y, cellSize, cellSize, QColor(240, 217, 181));
                else painter.fillRect(x, y, cellSize, cellSize, QColor(181, 136, 99));
            }
        }

        //Рамка вокруг игрового поля
        QPen pen(Qt::black);
        pen.setWidth(2);
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(margin, margin, boardSize, boardSize);

        //Отрисовка коня
        if (knightX != -1 && knightY != -1) {
            font.setPixelSize(cellSize * 0.7); // Размер символа
            painter.setFont(font);
            painter.setPen(Qt::black);

            int x = margin + knightX * cellSize;
            int y = margin + (7 - knightY) * cellSize;

            // Юникод коня
            painter.drawText(QRect(x, y, cellSize, cellSize), Qt::AlignCenter, "♞");
        }
    }

private:
    int knightX = -1;
    int knightY = -1;
};

//Главное окно

class MainWindow : public QWidget {
    Q_OBJECT

public:
    MainWindow() {
        setupUI();
        
        // Инициализация таймера анимации
        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &MainWindow::onTimerTick);
        
        // Обработка кнопки
        connect(btnStart, &QPushButton::clicked, this, &MainWindow::onStartClicked);

        setWindowTitle("Chess Knight Stage 2");
    }

private slots:
    // Нажатие кнопки Старт
    void onStartClicked() {
        lblMoves->setText(""); // Сброс счетчика

        QString s = inputStart->text().toLower();
        QString e = inputEnd->text().toLower();

        if (s.length() != 2 || e.length() != 2) {
            QMessageBox::warning(this, "Ошибка", "Формат: a1, h8");
            return;
        }

        Point start = { s[0].toLatin1() - 'a', s[1].toLatin1() - '1' };
        Point end = { e[0].toLatin1() - 'a', e[1].toLatin1() - '1' };

        // Поиск пути
        currentPath = findPath(start, end);

        if (currentPath.empty()) {
            QMessageBox::warning(this, "Ошибка", "Путь не найден!");
            return;
        }

        // Блокировка интерфейса во время анимации
        setControlsEnabled(false);
        
        // Установка коня на старт
        pathIndex = 0;
        board->setKnightPos(currentPath[0].x, currentPath[0].y);
        
        // Запуск таймера (500 мс)
        timer->start(500);
    }

    // Тик таймера (Анимация движения)
    void onTimerTick() {
        pathIndex++;
        
        // Если дошли до конца пути
        if (pathIndex >= currentPath.size()) {
            timer->stop();
            setControlsEnabled(true);
            
            // Вывод количества ходов в метку
            lblMoves->setText("Ходов: " + QString::number(currentPath.size() - 1));

            QMessageBox::information(this, "Успех", "Конь прибыл!");
            return;
        }
        
        // Перемещение коня
        board->setKnightPos(currentPath[pathIndex].x, currentPath[pathIndex].y);
    }

private:
    // Настройка элементов интерфейса
    void setupUI() {
        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        
        // Добавляем доску
        board = new ChessBoard();
        mainLayout->addWidget(board, 0, Qt::AlignCenter);

        // Панель управления (снизу)
        QHBoxLayout *controls = new QHBoxLayout();
        
        inputStart = new QLineEdit(); inputStart->setPlaceholderText("a1");
        inputEnd = new QLineEdit(); inputEnd->setPlaceholderText("h8");
        btnStart = new QPushButton("Старт");
        
        // Счетчик ходов
        lblMoves = new QLabel(""); 
        lblMoves->setFixedWidth(70);

        controls->addWidget(new QLabel("Start:"));
        controls->addWidget(inputStart);
        controls->addWidget(new QLabel("End:"));
        controls->addWidget(inputEnd);
        controls->addWidget(btnStart);
        controls->addWidget(lblMoves);

        mainLayout->addLayout(controls);
    }

    // Блокировка/Разблокировка кнопок
    void setControlsEnabled(bool en) {
        btnStart->setEnabled(en);
        inputStart->setEnabled(en);
        inputEnd->setEnabled(en);
    }

    ChessBoard *board;
    QLineEdit *inputStart;
    QLineEdit *inputEnd;
    QPushButton *btnStart;
    QLabel *lblMoves;
    QTimer *timer;
    std::vector<Point> currentPath;
    int pathIndex;
};

#include "main.moc"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow win;
    win.show();
    return app.exec();
}