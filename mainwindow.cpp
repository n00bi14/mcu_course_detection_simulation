#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    myPlayer = new Player();
    QObject::connect(myPlayer, SIGNAL(processedImage(QImage)), this, SLOT(updatePlayerUI(QImage)));
    ui->setupUi(this);
}
MainWindow::~MainWindow()
{
    // delete myPlayer;
    delete ui;
}

void MainWindow::updatePlayerUI(QImage img)
{
    if (!img.isNull())
    {
        ui->lblVideo->setAlignment(Qt::AlignCenter);
        ui->lblVideo->setPixmap(QPixmap::fromImage(img).scaled(ui->lblVideo->size(), Qt::KeepAspectRatio, Qt::FastTransformation));
        update = true;
        ui->sldTime->setValue(ui->sldTime->value() + 1);
        update = false;
    }
}

void MainWindow::on_btnLoad_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this,
                                          tr("Open Video"), ".",
                                          tr("Video Files (*.avi *.mpg *.mp4)"));
    if (!filename.isEmpty()){

        if (!myPlayer->loadVideo(filename.toStdString()))
        {
            QMessageBox msgBox;
            msgBox.setText("The selected video could not be opened!");
            msgBox.exec();
        }

        ui->sldTime->setMaximum(myPlayer->getFrameCount());
        ui->sldTime->setValue(0);
    }
}

void MainWindow::on_btnPlay_clicked()
{
    if (myPlayer->isStopped())
    {
        myPlayer->Play();
        ui->btnPlay->setText(tr("Stop"));
    }else
    {
        myPlayer->Stop();
        ui->btnPlay->setText(tr("Play"));
    }
}


void MainWindow::on_sldTime_valueChanged(int value)
{
    if(!update)
        myPlayer->goTo(value);
}
