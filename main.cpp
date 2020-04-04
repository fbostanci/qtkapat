#include "qtkapat.h"
#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Qtkapat w;
    w.show();
    return a.exec();
}
