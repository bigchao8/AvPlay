#include "caster.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //Caster w;
    //w.show();

    new Caster;
    return a.exec();
}
