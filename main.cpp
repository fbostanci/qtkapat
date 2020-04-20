#include "qtkapat.h"
#include <QApplication>
#include <QSharedMemory>

int main(int argc, char *argv[])
{

    QSharedMemory _singular("QtkapatInstance");
    if(_singular.attach(QSharedMemory::ReadOnly))
    {
        _singular.detach();
        return -42;
    } else {
        _singular.create(1);
    }
    QApplication a(argc, argv);

    Qtkapat w;
    w.show();
    return a.exec();
}
