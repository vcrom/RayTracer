#include "mainwindow.h"
#include "ui_mainwindow.h"

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

#include <QFileDialog>
void MainWindow::saveImage()
{
    auto fileName = QFileDialog::getSaveFileName(this,tr("Save Image"), "", tr("PPM (*.ppm)"));
    if(!fileName.isNull())
    {
        ui->image->saveImage(fileName);
    }
}

void MainWindow::renderImage()
{
    ui->image->renderImage();
}
