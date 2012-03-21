#include "raydisplaywindow.h"
#include "ui_raydisplaywindow.h"

RayDisplayWindow::RayDisplayWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::RayDisplayWindow)
{
    ui->setupUi(this);
}

RayDisplayWindow::~RayDisplayWindow()
{
    delete ui;
}
