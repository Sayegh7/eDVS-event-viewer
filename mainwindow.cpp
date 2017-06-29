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
    for(int i = 0; i < 180; ++i){
        houghMatrix[i] = new int[364];
        std::fill_n(houghMatrix[i], 364, 0);

    }



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
//    openSerialPort();
//    QByteArray x;
//    x = QString("E+\nE+").toUtf8();
//    this->writeData(x);

    for(int length = 0; length < 20; length++){
        image->setPixel(100+length, 100+length, qRgb(255, 255, 255));
    }
    for(int a = 0; a < 20; a++){
        image->setPixel(100+a, 100-a, qRgb(255, 255, 255));
        std::cout << 100+a << " " << 100-a << std::endl;
    }

    readData();


}

double degrees_to_radians(int degrees){
    return degrees * (PI) / 180.0;
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

//    QByteArray data = serial->readAll();
//    char x; char polarity;
//    char y; char validEvent;

//    for(int a = 0; a < data.length(); a+=2){
//     validEvent = (data[a] & 0x80) >> 7;
//     if(validEvent == 0){
//        continue;
//     }
//     int r;

//     y = data[a] & 0x7f;
//     x = data[a+1] & 0x7f;
//     polarity = (data[a+1] & 0x80) >> 7;
//     if((int) polarity == 0){
//        image->setPixel((int)x, (int)y, qRgb(255, 0, 0));

//     }else{
//        image->setPixel((int)x, (int)y, qRgb(0, 0, 255));

//     }

//     for(int angle = 0; angle < 180; angle++){
//         r = (int)x * cos(angle) + (int)y * sin(angle);
//         int radius = std::abs(r);
//              houghMatrix[angle][radius]++;
//     }



//    }

    int r;
    int x = 51;
    for(int coordinate = 100; coordinate < 120; coordinate++){
        for(int angle = 0; angle < 180; angle+=45){
            r = coordinate * cos(degrees_to_radians(angle)) + coordinate * sin(degrees_to_radians(angle));

            int radius = r + 182;
            std::cout << houghMatrix[angle][radius] << std::endl;

            houghMatrix[angle][radius]++;
            std::cout << "angle: " << angle << ", r: " << radius-182 << std::endl;
        }
    }

    for(int coordinate = 100; coordinate < 120; coordinate++){
        x--;
        for(int angle = 0; angle < 180; angle+=45){
            r = coordinate * cos(degrees_to_radians(angle)) + x * sin(degrees_to_radians(angle));
            int radius = r + 182;
//            std::cout << houghMatrix[angle][radius] << std::endl;

            houghMatrix[angle][radius]++;
            std::cout << "angle: " << angle << ", r: " << radius-182 << std::endl;
        }
    }





    QPixmap pixmap(1,1);
    pixmap.convertFromImage(*image);

            // 0 degrees start at top right
    //90 degrees at bottom left
//                    int row = 135;
//                    int col = 182+70;
//        drawLine(col, row);

//    draw lines
    for(int row = 0; row < 180; row++){
        for(int col= 0; col< 364; col++){
            if(houghMatrix[row][col] >= 20 ){
                //candidate line
                int maxLength = 182;
                int angleOffset = 90;
                double px = (col-maxLength) * round(cos(degrees_to_radians(row + angleOffset)));
                double py = (col-maxLength)* round(sin(degrees_to_radians(row + angleOffset)));
                double xBefore = (col-maxLength) * round(cos(degrees_to_radians(row)));
                double yBefore = (col-maxLength)* round(sin(degrees_to_radians(row)));

                if(std::abs(xBefore) == std::abs(px)){
                    xBefore = 0;
                }
                if(std::abs(yBefore) == std::abs(py)){
                    yBefore = 0;
                }

                double p1_x = px + maxLength * round(cos(degrees_to_radians(row + angleOffset))) + xBefore;
                double p1_y = py + maxLength * round(sin(degrees_to_radians(row + angleOffset))) + yBefore;
                double p2_x = px - maxLength * round(cos(degrees_to_radians(row + angleOffset))) + xBefore;
                double p2_y = py - maxLength * round(sin(degrees_to_radians(row + angleOffset))) + yBefore;
                std::cout << "Angle: " << row+90 << " Radius: " << col-maxLength << std::endl;
                std::cout << "P1X: " << p1_x << " P1Y: " << p1_y << std::endl;
                std::cout << "P2X: " << p2_x << " P2Y: " << p2_y << std::endl;
                QPainter p (&pixmap);
//                std::cout << houghMatrix[row][col] << std::endl;
                p.setPen (Qt::green);
                p.drawLine (p1_x, p1_y, p2_x, p2_y);
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

void MainWindow::drawLine(int angle, int radius){

    QPixmap pixmap(1,1);
    pixmap.convertFromImage(*image);

    int maxLength = 182;
    int angleOffset = 90;
    double px = (radius-maxLength) * round(cos(degrees_to_radians(angle + angleOffset)));
    double py = (radius-maxLength)* round(sin(degrees_to_radians(angle + angleOffset)));
    double xBefore = (radius-maxLength) * round(cos(degrees_to_radians(angle)));
    double yBefore = (radius-maxLength)* round(sin(degrees_to_radians(angle)));

//                    if(std::abs(xBefore) == std::abs(px)){
//                        xBefore = 0;
//                    }
//                    if(std::abs(yBefore) == std::abs(py)){
//                        yBefore = 0;
//                    }

    double p1_x = px + maxLength * round(cos(degrees_to_radians(angle + angleOffset))) + xBefore;
    double p1_y = py + maxLength * round(sin(degrees_to_radians(angle + angleOffset))) + yBefore;
    double p2_x = px - maxLength * round(cos(degrees_to_radians(angle + angleOffset))) + xBefore;
    double p2_y = py - maxLength * round(sin(degrees_to_radians(angle + angleOffset))) + yBefore;
    std::cout << "Angle: " << angle+angleOffset << " Radius: " << radius-maxLength << std::endl;
    std::cout << "Before: " << xBefore << ", " << yBefore << std::endl;
    std::cout << "P1X: " << p1_x << " P1Y: " << p1_y << std::endl;
    std::cout << "P2X: " << p2_x << " P2Y: " << p2_y << std::endl;
    QPainter p (&pixmap);
//                std::cout << houghMatrix[angle][col] << std::endl;
    p.setPen (Qt::green);
    p.drawLine (p1_x, p1_y, p2_x, p2_y);
    p.end ();

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
