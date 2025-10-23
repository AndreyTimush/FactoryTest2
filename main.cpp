#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QtConcurrent>
#include <QFuture>
#include <QMutex>

struct ChunkResult {
    qint64 sum = 0;
    qint64 diff = 0;
    int xorResult = 0;
    bool isFirst = true;
};

ChunkResult processChunk(const QStringList &parts) {
    ChunkResult res;
    for (const QString &p : parts) {
        bool ok;
        qint64 number = p.toLongLong(&ok);
        if (!ok) continue;

        if (res.isFirst) {
            res.diff = number;
            res.isFirst = false;
        } else {
            res.diff -= number;
        }

        res.sum += number;
        res.xorResult ^= static_cast<int>(number);
    }
    return res;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QFile file("numbers.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Не удалось открыть файл!";
        return -1;
    }

    QTextStream in(&file);
    const int SIZE_CHUNK = 1024 * 1024; // Размер чанка
    QString buffer; // Буфер для неполных чисел на границе чанка

    QList<QFuture<ChunkResult>> futures;

    while (!in.atEnd()) {
        // Читаем блок данных 1МБ
        QString chunk = buffer + in.read(SIZE_CHUNK);
        // Находим последний пробел/перевод строки
        int lastSpace = chunk.lastIndexOf(QRegExp("\\s"));

        // Если нашли, сохраняем остаток в буфер
        if (lastSpace != -1 && !in.atEnd()) {
            buffer = chunk.mid(lastSpace + 1);
            chunk = chunk.left(lastSpace);
        } else {
            buffer.clear();
        }

        // Парсим числа из текущего чанка
        QStringList parts = chunk.split(QRegExp("\\s+"), Qt::SkipEmptyParts);

        futures.append(QtConcurrent::run(processChunk, parts));
    }

    file.close();

    qint64 totalSum = 0;
    qint64 totalDiff = 0;
    int totalXor = 0;
    bool firstChunk = true;

    for (auto &f : futures) {
        ChunkResult r = f.result();
        totalSum += r.sum;
        totalXor ^= r.xorResult;
        if (firstChunk) {
            totalDiff = r.diff;
            firstChunk = false;
        } else {
            totalDiff -= r.sum;
        }
    }

    qDebug() << "Результат суммы всех чисел:" << totalSum;
    qDebug() << "Результат вычитания из первого числа:" << totalDiff;
    qDebug() << "Результат XOR:" << totalXor;

    return 0;
}
