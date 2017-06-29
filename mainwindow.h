#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QtSerialPort/QSerialPort>
#include <QTextStream>
#include <QObject>
#include <QFile>
#include <QGraphicsPixmapItem>
#include <ctime>

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
    void openSerialPort();
    void closeSerialPort();
    void writeData(const QByteArray &data);
    void readData();
    void delay();
    void drawLine(int, int);
    void clearHoughMatrix();
    void handleError(QSerialPort::SerialPortError error);


private:
    Ui::MainWindow *ui;
    QSerialPort *serial;
    QGraphicsView *view;
    QGraphicsScene *scene;
    QImage *image;
    QGraphicsPixmapItem *pixItem;
    int previousTime;
    int resolution;
    int scale;
    int bufferingTime;
    QFile *file;
    int** houghMatrix;
};

#endif // MAINWINDOW_H
