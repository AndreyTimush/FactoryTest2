#include <QCoreApplication>
#include <QFile>
#include <QDebug>
#include <QtConcurrent>
#include <QFuture>
#include <QThreadPool>

struct ChunkResult {
    qint64 sum = 0;
    qint64 diff = 0;
    quint64 xorResult = 0;
    bool isFirst = true;
};

ChunkResult processChunk(const QByteArray &chunkData) {
    ChunkResult res;
    qint64 number = 0;
    bool hasNum = false;

    for (char c : chunkData) {
        if (c >= '0' && c <= '9') {
            number = number * 10 + (c - '0');
            hasNum = true;
        } else if (hasNum) {
            res.sum += number;
            res.xorResult ^= static_cast<quint64>(number);
            number = 0;
            hasNum = false;
        }
    }

    if (hasNum) {
        res.sum += number;
        res.xorResult ^= static_cast<quint64>(number);
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

    QThreadPool::globalInstance()->setMaxThreadCount(3);

    const qint64 CHUNK_SIZE = 1024 * 1024; // Размер чанка
    QByteArray buffer; // Буфер для неполных чисел на границе чанка
    buffer.reserve(CHUNK_SIZE);

    QList<QFuture<ChunkResult>> futures;

    qint64 firstNumber = 0;
    bool firstNumberFound = false;

    while (!file.atEnd()) {
        QByteArray chunk = buffer + file.read(CHUNK_SIZE);
        qint64 lastSpace = chunk.lastIndexOf(' ');

        if (!firstNumberFound) {
            qint64 number = 0;
            bool hasNum = false;
            for (char c : chunk) {
                if (c >= '0' && c <= '9') {
                    number = number * 10 + (c - '0');
                    hasNum = true;
                } else if (hasNum) {
                    firstNumber = number;
                    firstNumberFound = true;
                    break; // нашли первое число — выходим
                }
            }
            if (!firstNumberFound && hasNum) {
                firstNumber = number;
                firstNumberFound = true;
            }
        }

        if (lastSpace != -1 && !file.atEnd()) {
            buffer = chunk.mid(lastSpace + 1);
            chunk.truncate(lastSpace + 1);
        } else {
            buffer.clear();
        }

        futures.append(QtConcurrent::run(processChunk, chunk));
    }

    file.close();

    qint64 totalSum = 0;
    qint64 totalDiff = firstNumber;
    quint64 totalXor = 0;

    for (auto &f : futures) {
        ChunkResult r = f.result();
        totalSum += r.sum;
        totalXor ^= r.xorResult;
        totalDiff -= r.sum;
    }

    totalDiff += firstNumber;

    qDebug() << "Результат суммы всех чисел:" << totalSum;
    qDebug() << "Результат вычитания из первого числа:" << totalDiff;
    qDebug() << "Результат XOR:" << totalXor;

    return 0;
}
