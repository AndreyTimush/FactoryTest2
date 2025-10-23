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
    QList<int> numbers;

    // Читаем все числа из файла
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split(QRegExp("\\s+"), Qt::SkipEmptyParts);
        for (const QString &p : parts)
            numbers.append(p.toInt());
    }

    file.close();

    if (numbers.isEmpty()) {
        qWarning() << "Файл пустой или не содержит чисел!";
        return -1;
    }

    // Сумма
    int sum = 0;
    for (int n : numbers)
        sum += n;

    // Из первого числа вычитаем все остальные
    int diff = numbers[0];
    for (int i = 1; i < numbers.size(); ++i) {
        qDebug() << numbers[i];
        diff -= numbers[i];
    }

    // XOR
    int x = 0;
    for (int n : numbers)
        x ^= n;

    qDebug() << "Результат суммы всех чисел:" << sum;
    qDebug() << "Результат вычитания из первого числа:" << diff;
    qDebug() << "Результат XOR:" << x;

    return 0;
}
