#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QFile file("numbers.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Не удалось открыть файл!";
        return -1;
    }

    QTextStream in(&file);

    // Переменные для накопления результатов
    qint64 sum = 0;
    qint64 diff = 0;
    int xorResult = 0;
    bool isFirstNumber = true;
    bool hasNumbers = false;

    const int SIZE_CHUNK = 1024 * 1024; // Размер чанка
    QString buffer; // Буфер для неполных чисел на границе чанка

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

        for (const QString &p : parts) {
            bool ok;
            qint64 number = p.toLongLong(&ok);

            if (!ok) continue;

            hasNumbers = true;

            // Сумма
            sum += number;

            // Разность
            if (isFirstNumber) {
                diff = number;
                isFirstNumber = false;
            } else {
                diff -= number;
            }

            // XOR
            xorResult ^= static_cast<int>(number);
        }
    }

    // Обрабатываем остаток в буфере
    if (!buffer.isEmpty()) {
        QStringList parts = buffer.split(QRegExp("\\s+"), Qt::SkipEmptyParts);
        for (const QString &p : parts) {
            bool ok;
            qint64 number = p.toLongLong(&ok);

            if (!ok) continue;

            hasNumbers = true;
            sum += number;

            if (isFirstNumber) {
                diff = number;
                isFirstNumber = false;
            } else {
                diff -= number;
            }

            xorResult ^= static_cast<int>(number);
        }
    }

    file.close();

    if (!hasNumbers) {
        qWarning() << "Файл пустой или не содержит чисел!";
        return -1;
    }

    qDebug() << "Результат суммы всех чисел:" << sum;
    qDebug() << "Результат вычитания из первого числа:" << diff;
    qDebug() << "Результат XOR:" << xorResult;

    return 0;
}
