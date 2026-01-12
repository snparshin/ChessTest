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

// --- ЛОГИКА (БЕЗ ИЗМЕНЕНИЙ) ---

struct Point {
    int x, y;
};

bool isValid(int x, int y) {
    return (x >= 0 && x < 8 && y >= 0 && y < 8);
}

std::vector<Point> findPath(Point start, Point end) {
    if (!isValid(start.x, start.y) || !isValid(end.x, end.y)) return {};
    if (start.x == end.x && start.y == end.y) return {start};

    int dx[] = { -2, -1, 1, 2, 2, 1, -1, -2 };
    int dy[] = { 1, 2, 2, 1, -1, -2, -2, -1 };

    Point parent[8][8];
    bool visited[8][8];

    for(int i=0; i<8; i++) {
        for(int j=0; j<8; j++) {
            visited[i][j] = false;
            parent[i][j] = {-1, -1};
        }
    }

    std::queue<Point> q;
    q.push(start);
    visited[start.x][start.y] = true;

    bool found = false;

    while (!q.empty()) {
        Point curr = q.front();
        q.pop();

        if (curr.x == end.x && curr.y == end.y) {
            found = true;
            break;
        }

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

// --- GUI (С ИСПРАВЛЕНИЯМИ) ---

class ChessBoard : public QWidget {
    Q_OBJECT

public:
    ChessBoard(QWidget *parent = nullptr) : QWidget(parent) {
        // ИСПРАВЛЕНО: Увеличили размер окна до 460, чтобы влезли нижние и правые отступы
        setFixedSize(460, 460); 
    }

    void setKnightPos(int x, int y) {
        knightX = x;
        knightY = y;
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::TextAntialiasing);

        int margin = 30;     // Отступ (слева, сверху, снизу, справа)
        int boardSize = 400; // Размер игровой зоны
        int cellSize = boardSize / 8;

        // 1. Рисуем координаты (Буквы и Цифры)
        painter.setPen(Qt::black);
        QFont font = painter.font();
        font.setBold(true);
        font.setPixelSize(14);
        painter.setFont(font);

        for (int i = 0; i < 8; ++i) {
            QString letter = QChar('a' + i);
            int x = margin + i * cellSize;
            
            // Буквы снизу: рисуем их в прямоугольнике высотой 20px сразу под доской
            // (+5 пикселей отступа от доски, чтобы не прилипали)
            painter.drawText(QRect(x, margin + boardSize, cellSize, 25), Qt::AlignCenter, letter);
            
            QString number = QString::number(i + 1);
            int y = margin + (7 - i) * cellSize;
            // Цифры слева
            painter.drawText(QRect(0, y, margin - 5, cellSize), Qt::AlignRight | Qt::AlignVCenter, number);
        }

        // 2. Рисуем клетки
        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {
                int x = margin + i * cellSize;
                int y = margin + (7 - j) * cellSize;

                if ((i + j) % 2 == 0) painter.fillRect(x, y, cellSize, cellSize, QColor(240, 217, 181));
                else painter.fillRect(x, y, cellSize, cellSize, QColor(181, 136, 99));
            }
        }

        // 3. Рамка вокруг доски (жирная)
        QPen pen(Qt::black);
        pen.setWidth(2);
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(margin, margin, boardSize, boardSize);

        // 4. Конь
        if (knightX != -1 && knightY != -1) {
            font.setPixelSize(cellSize * 0.7);
            painter.setFont(font);
            painter.setPen(Qt::black);

            int x = margin + knightX * cellSize;
            int y = margin + (7 - knightY) * cellSize;

            painter.drawText(QRect(x, y, cellSize, cellSize), Qt::AlignCenter, "♞");
        }
    }

private:
    int knightX = -1;
    int knightY = -1;
};

class MainWindow : public QWidget {
    Q_OBJECT

public:
    MainWindow() {
        setupUI();
        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &MainWindow::onTimerTick);
        connect(btnStart, &QPushButton::clicked, this, &MainWindow::onStartClicked);

        setWindowTitle("Chess Knight Stage 2");
    }

private slots:
    void onStartClicked() {
        // Очищаем счетчик перед новым стартом
        lblMoves->setText("");

        QString s = inputStart->text().toLower();
        QString e = inputEnd->text().toLower();

        if (s.length() != 2 || e.length() != 2) {
            QMessageBox::warning(this, "Ошибка", "Формат: a1, h8");
            return;
        }

        Point start = { s[0].toLatin1() - 'a', s[1].toLatin1() - '1' };
        Point end = { e[0].toLatin1() - 'a', e[1].toLatin1() - '1' };

        currentPath = findPath(start, end);

        if (currentPath.empty()) {
            QMessageBox::warning(this, "Ошибка", "Путь не найден!");
            return;
        }

        setControlsEnabled(false);
        pathIndex = 0;
        board->setKnightPos(currentPath[0].x, currentPath[0].y);
        timer->start(500);
    }

    void onTimerTick() {
        pathIndex++;
        if (pathIndex >= currentPath.size()) {
            timer->stop();
            setControlsEnabled(true);
            
            // ДОБАВЛЕНО: Показываем количество ходов по прибытии
            // Количество ходов = количество точек в пути минус 1 (начальная позиция)
            lblMoves->setText("Ходов: " + QString::number(currentPath.size() - 1));

            QMessageBox::information(this, "Успех", "Конь прибыл!");
            return;
        }
        board->setKnightPos(currentPath[pathIndex].x, currentPath[pathIndex].y);
    }

private:
    void setupUI() {
        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        board = new ChessBoard();
        mainLayout->addWidget(board, 0, Qt::AlignCenter);

        QHBoxLayout *controls = new QHBoxLayout();
        
        inputStart = new QLineEdit(); inputStart->setPlaceholderText("a1");
        inputEnd = new QLineEdit(); inputEnd->setPlaceholderText("h8");
        btnStart = new QPushButton("Старт");
        // ДОБАВЛЕНО: Метка для счетчика ходов
        lblMoves = new QLabel(""); 
        lblMoves->setFixedWidth(70); // Фиксированная ширина, чтобы не прыгало

        controls->addWidget(new QLabel("Start:"));
        controls->addWidget(inputStart);
        controls->addWidget(new QLabel("End:"));
        controls->addWidget(inputEnd);
        controls->addWidget(btnStart);
        // Добавляем счетчик на панель
        controls->addWidget(lblMoves);

        mainLayout->addLayout(controls);
    }

    void setControlsEnabled(bool en) {
        btnStart->setEnabled(en);
        inputStart->setEnabled(en);
        inputEnd->setEnabled(en);
    }

    ChessBoard *board;
    QLineEdit *inputStart;
    QLineEdit *inputEnd;
    QPushButton *btnStart;
    QLabel *lblMoves; // Новая метка
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