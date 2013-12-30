#include "mainwindow.h"
#include "ui_mainwindow.h"

//THIS IS WHERE I STOPPED
//it's not able to find ui_mainwindow.h

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}
