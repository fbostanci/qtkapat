#include "qtkapat.h"
#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Qtkapat q;
    q.show();
    return a.exec();
}
