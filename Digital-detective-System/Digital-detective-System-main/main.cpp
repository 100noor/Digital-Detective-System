#include "Login.h"
#include "logic.h"
#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);   // ← MUST be first

    initializeSystem();            // ← AFTER QApplication

    Login w;
    w.show();
    return a.exec();
}