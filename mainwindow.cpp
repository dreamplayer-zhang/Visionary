#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "filter.h"
#include <QFileDialog>
#include <QPixmap>
#include <QMessageBox>
#include <QInputDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    imageType = V_NORMAL;
    qlabel = new QLabel(ui->label);
    originalImage = NULL;
    setWindowFlags(windowFlags()
                   &~Qt::WindowMaximizeButtonHint);    // 禁止最大化按钮
    setFixedSize(this->width(),this->height());                     // 禁止拖动窗口大小
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showResponseTime()
{
    QString s = QString("%1s").arg(F_responseTime());
    ui->responseTime->setText(s);
}

void MainWindow::showImage(QImage *image)
{
    if (currentImage != originalImage)
        free(currentImage);
    currentImage = image;
    qlabel->setPixmap(QPixmap::fromImage(autoscale()));
}

QImage MainWindow::autoscale()
{
    QImage newImage= currentImage->scaled(PIC_HEIGHT, PIC_WIDTH,Qt::KeepAspectRatio, Qt::SmoothTransformation);
    return newImage;
}

void MainWindow::on_actionOpen_triggered()
{
    // open file

    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("打开图片"),
                                                    "F:\\PersonalThings\\Artirea\\Pictures\\素材",
                                                    "Images (*.png *.bmp *.jpg *.jpeg)");

    //QString fileName = "F:\\PersonalThings\\Artirea\\Pictures\\素材\\test.png";

    if (fileName == "" || fileName == NULL) {
        return;
    }
    QImage *image = new QImage;
    if(!image->load(fileName)) {
        QMessageBox::information(this, tr("打开图像失败"), tr("打开图像失败"));
    }
    originalImage = image;
    currentImage = image;

    // scale to appropriate size
    QImage scaledImage = autoscale();

    // log
    currentFile = fileName;
    qlabel->setPixmap(QPixmap::fromImage(scaledImage));
    qlabel->resize(PIC_HEIGHT, PIC_WIDTH);
    qlabel->setAlignment(Qt::AlignCenter);
    ui->menuFilter->setEnabled(true);
}

void MainWindow::on_actionRecover_triggered()
{
    if (originalImage!=NULL) {
        showImage(originalImage);
        imageType = V_NORMAL;
    }
}

void MainWindow::on_actionExit_triggered()
{
    QApplication* app;
    app->exit(0);
}

void MainWindow::on_actionSave_as_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("保存为"),
                                                    "C:\\Users\\Administrator\\Desktop",
                                                    "Images (*.png *.bmp *.jpg *.jpeg)");
    currentImage->save(fileName);
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, "关于Visionary",
                       "Visionary是一个强大的图像处理软件。");
}

void MainWindow::on_actionDecoloration_triggered()
{
    if (imageType == V_GREY) {
        return;
    }
    showImage(F_decolor(currentImage));
    showResponseTime();
    imageType = V_GREY;
}

void MainWindow::on_actionBinarization_triggered()
{
    bool ok = false;
    int threshold = QInputDialog::getInt(this,tr("Visionary"),
                                         tr("请输入阈值"),
                                         0, 0, 255,
                                         1, &ok);
    if (!ok)
        return;
    showImage(F_binarization(currentImage, threshold));
    showResponseTime();
    imageType = V_BINARY;
}

void MainWindow::on_actionblur_triggered()
{
    showImage(F_blur(currentImage));
    showResponseTime();
}

void MainWindow::on_actionsharpen_triggered()
{
    showImage(F_sharpen(currentImage));
    showResponseTime();
}

void MainWindow::on_actiondilation_triggered()
{
    showImage(F_dilation(currentImage));
    showResponseTime();
}

void MainWindow::on_actionerosion_triggered()
{
    showImage(F_erosion(currentImage));
    showResponseTime();
}