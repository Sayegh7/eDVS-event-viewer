#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtGui>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPixmap>
#include <QtSerialPort/QSerialPort>
#include <chrono>
#include <iostream>
#include <math.h>
#define PI 3.14159265



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    serial = new QSerialPort(this);
    QThread *thread = new QThread();
    serial->moveToThread(thread);
    connect(serial, static_cast<void (QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error),
            this, &MainWindow::handleError);

    connect(serial, &QSerialPort::readyRead, this, &MainWindow::readData);

    thread->start();

    bufferingTime = 16;
    previousTime = 0;
    resolution = 128;
    scale = 5;
    houghMatrix = new int*[180];
    for(int i = 0; i < 180; ++i)
        houghMatrix[i] = new int[364];



    image = new QImage(this->resolution, this->resolution, QImage::Format_RGB32);
    QRgb value;


    for(int i = 0; i < this->resolution; i++){
        for(int j = 0; j <this->resolution; j++){
            value = qRgb(127, 127, 127); // 0xffbd9527
            this->image->setPixel(i, j, value);
        }
    }
    QString filename="Data.txt";
    file = new QFile( filename );

    QPixmap pixmap(1,1);
    pixmap.convertFromImage(*image);
    pixmap = pixmap.scaled(this->resolution*this->scale,this->resolution*this->scale);
    scene = new QGraphicsScene();
    pixItem = new QGraphicsPixmapItem(pixmap);
    pixItem->setPixmap(pixmap);
    scene->addItem(pixItem);
    view = new QGraphicsView();

    view->setScene(scene);
    this->setCentralWidget(view);
    openSerialPort();
    QByteArray x;
    x = QString("E+\nE+").toUtf8();
    this->writeData(x);


}

void MainWindow::clearHoughMatrix(){
    memset(houghMatrix, 0, sizeof(houghMatrix[0][0]) * 180 * 364);
}

void MainWindow::openSerialPort()
{
    //    serial->setPortName("ttyUSB0");
    serial->setPortName("COM3");
    serial->setBaudRate(12000000);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::HardwareControl);
    if (serial->open(QIODevice::ReadWrite)) {



    } else {
    }
}

void MainWindow::writeData(const QByteArray &data)
{
    serial->write(data);
}

QString toString(int Number){
    return QString::number(Number);
}

void MainWindow::delay()
{
    QTime dieTime= QTime::currentTime().addSecs(3);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

void MainWindow::closeSerialPort()
{
    if (serial->isOpen())
        serial->close();
    QTextStream out(stdout);

    out << ("Disconnected");
}



void MainWindow::readData()
{

    QByteArray data = serial->readAll();

    char x; char polarity;
    char y; char validEvent;

    for(int a = 0; a < data.length(); a+=2){
     validEvent = (data[a] & 0x80) >> 7;
     if(validEvent == 0){
        continue;
     }
     y = data[a] & 0x7f;
     x = data[a+1] & 0x7f;
     polarity = (data[a+1] & 0x80) >> 7;
     int r;
     if((int) polarity == 0){
        image->setPixel((int)x, (int)y, qRgb(255, 0, 0));

     }else{
        image->setPixel((int)x, (int)y, qRgb(0, 0, 255));

     }

     for(int angle = 0; angle < 180; angle++){
         r = (int)x * cos(angle) + (int)y * sin(angle);
         int radius = std::abs(r);
              houghMatrix[angle][radius]++;
     }



    }




    QPixmap pixmap(1,1);
    pixmap.convertFromImage(*image);


//    draw lines
    for(int row = 0; row < 180; row++){
        for(int col= 0; col< 364; col++){
            if(houghMatrix[row][col] > 30 && houghMatrix[row][col] < 182){
                int maxLength = 182;
                double px = (col) * cos(row);
                double py = (col)* sin(row);
                double p1_x = px + maxLength * cos(row);
                double p1_y = py + maxLength * sin(row);
                double p2_x = px - maxLength * cos(row);
                double p2_y = py - maxLength * sin(row);

                QPainter p (&pixmap);
                p.setPen (Qt::green);
//                                p.drawLine ((int)p1_x, (int)p1_y, (int)p2_x, (int)p2_y);
                                p.drawLine (p1_x, p1_y, px, py);
//                this->statusBar()->showMessage(QString("%1").arg(p2_y));
                p.end ();
            }
            houghMatrix[row][col] = 0;

        }
    }


    pixmap = pixmap.scaled(this->resolution*this->scale,this->resolution*this->scale);
    pixItem->setPixmap(pixmap);
    image = new QImage(this->resolution, this->resolution, QImage::Format_RGB32);
    image->fill(QColor(127, 127, 127, 255));






}


void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QTextStream out(stdout);

        out << serial->errorString();
        closeSerialPort();
    }
}




MainWindow::~MainWindow()
{
    delete ui;
}
