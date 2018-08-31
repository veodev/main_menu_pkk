#include "widget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget * w = new Widget;
    qApp->setOverrideCursor(QCursor(Qt::BlankCursor));
    w->showFullScreen();

    return a.exec();
    delete w;
}
