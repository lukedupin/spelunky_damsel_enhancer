#include <QCoreApplication>
#include "spelunky.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    //Tell the user how to use the program
    if (argc < 2)
    {
        qDebug( "usage: %s pid [damsel_hearts]\n", argv[0]);
        return 1;
    }

    //Load my spelunky object and we go!
    Spelunky spelunky( argv[1], (argc > 2)? argv[2]: "4" );

    return a.exec();
}
