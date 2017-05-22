#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtGui>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPixmap>
#include <QtSerialPort/QSerialPort>
#include <chrono>
#include <iostream>



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

    image = new QImage(this->resolution, this->resolution, QImage::Format_RGB32);
    QRgb value;


    for(int i = 0; i < this->resolution; i++){
        for(int j = 0; j <this->resolution; j++){
            value = qRgb(127, 127, 127); // 0xffbd9527
            this->image->setPixel(i, j, value);
        }
    }
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



void MainWindow::readData()
{

    QByteArray data = serial->readAll();


    char x; char polarity;
    char y; char validEvent;
//    std::cout << data.length()<< std::endl;
    for(int a = 0; a < data.length(); a+=2){
     validEvent = (data[a] & 0x80) >> 7;
     if(validEvent == 0){
        continue;
     }
     y = data[a] & 0x7f;
     x = data[a+1] & 0x7f;
     polarity = (data[a+1] & 0x80) >> 7;


     if((int) polarity == 0){
        image->setPixel((int)x, (int)y, qRgb(0, 0, 0));
     }else{
        image->setPixel((int)x, (int)y, qRgb(255, 255, 255));
     }



    }

    QPixmap pixmap(1,1);
    pixmap.convertFromImage(*image);
    pixmap = pixmap.scaled(this->resolution*this->scale,this->resolution*this->scale);
    pixItem->setPixmap(pixmap);
    image = new QImage(this->resolution, this->resolution, QImage::Format_RGB32);
    image->fill(QColor(127, 127, 127, 255));


//    if(data.length()>=2){
//    }else{
//        x = 0;
//        polarity = 0;
//    }


//    QTextStream out(stdout);
//    QString xString = toString(x);
//    QString yString = toString(y);
//    QString pString = toString(polarity);
//    out << "Y: " << (int)y << ' ' << "X: " << (int)x << ' ' << "Polarity: " << (int)polarity << endl;


    //EXP

//    std::cout << now << std::endl;
//    if(previousTime == 0){
//        previousTime = now;
//    }

//    if(now - previousTime <= this->bufferingTime){
//        if((int) polarity == 0){
//           image->setPixel((int)x, (int)y, qRgb(0, 0, 0));
//        }else{
//           image->setPixel((int)x, (int)y, qRgb(255, 255, 255));
//        }

//    }else{
//        if((int) polarity == 0){
//           image->setPixel((int)x, (int)y, qRgb(0, 0, 0));
//        }else{
//           image->setPixel((int)x, (int)y, qRgb(255, 255, 255));
//        }

//        QPixmap pixmap(1,1);
//        pixmap.convertFromImage(*image);
//        pixmap = pixmap.scaled(this->resolution*this->scale,this->resolution*this->scale);
//        pixItem->setPixmap(pixmap);
//        previousTime = now;
//        image = new QImage(this->resolution, this->resolution, QImage::Format_RGB32);
//        image->fill(QColor(127, 127, 127, 255));

//    }





}


void MainWindow::closeSerialPort()
{
    if (serial->isOpen())
        serial->close();
    QTextStream out(stdout);

    out << ("Disconnected");
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
