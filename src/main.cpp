#include "mainwindow.cpp"

#include <iostream>

int
main(int argc, char *argv[])
{
        QApplication app(argc, argv);
        MainWindow *window = new MainWindow;

        window->show();

        return app.exec();
}
