#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QString>
#include <vector>
#include "player.h"
#include "line.h"

using namespace std;
using namespace cv;

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
    //Display video frame in player UI
    void updatePlayerUI(QImage img);
    //Slot for the load video push button.
    void on_btnLoad_clicked();
    // Slot for the play push button.
    void on_btnPlay_clicked();

    void on_sldTime_valueChanged(int value);

private:
    Ui::MainWindow *ui;
    Player* myPlayer;
    bool update = false;

};

#endif // MAINWINDOW_H
