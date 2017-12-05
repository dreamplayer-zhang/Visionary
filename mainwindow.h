#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#define PIC_HEIGHT (860.0)
#define PIC_WIDTH  (640.0)

#include <QMainWindow>
#include <QLabel>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionOpen_triggered();

    void on_actionDecoloration_triggered();

    void on_actionRecover_triggered();

    void on_actionExit_triggered();

    void on_actionSave_as_triggered();

    void on_actionAbout_triggered();

    void on_actionBinarization_triggered();

    void on_actionblur_triggered();

    void on_actionsharpen_triggered();

    void on_actiondilation_triggered();

    void on_actionerosion_triggered();

private:
    Ui::MainWindow *ui;
    QString currentFile;
    QImage *originalImage;
    QImage *currentImage;
    QLabel *qlabel;
    enum {V_NORMAL, V_GREY, V_BINARY} imageType;

    void showResponseTime();
    void showImage(QImage *image);
    QImage autoscale();
};

#endif // MAINWINDOW_H