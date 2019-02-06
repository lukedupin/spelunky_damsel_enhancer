#ifndef SPELUNKY_H
#define SPELUNKY_H

#include <QObject>
#include <QTimer>
#include <QTime>
#include <QFile>

class Spelunky : public QObject
{
    Q_OBJECT

    private:
    QTimer _timer;
    QTime _delay;
    QString _pid;
    qint64 _address = 0;
    int _delayTime = 0;
    int _hearts = -1;
    int _boost = 1;
    int _resetBoost = 1;
    bool _cumulative = false;
    bool _capturedDamsel = false;
    bool _assignDamsel = false;

    QFile _mem;

    public:
    explicit Spelunky(const char* pid, const char* boost, QObject *parent = nullptr);

    signals:

    private:
    bool lostLink();

    bool findAddress();

    bool followAddress();

    public slots:
    void run();
};

//0x00B779D0

#endif // SPELUNKY_H
