#ifndef PLAYER_H
#define PLAYER_H

#include <QMutex>
#include <QThread>
#include <QImage>
#include <QWaitCondition>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <vector>
#include "player.h"
#include "line.h"

using namespace cv;

class Player : public QThread
{    Q_OBJECT

 private:
    bool stop;
    QMutex mutex;
    QWaitCondition condition;
    Mat frame;
    int frameRate;
    VideoCapture capture;
    Mat RGBframe;
    QImage img;
    void processFrame(Mat &frame);
    void getFrame(double second, VideoCapture& cap, Mat& frame);
    int videoStart;
    int cannyThreshold;
    bool lineInRegionOfIntrest(Rect& roi, Vec4i& line);

 signals:
 //Signal to output frame to be displayed
    void processedImage(const QImage &image);

 protected:
    void run();
    void msleep(int ms);

 public:
    //Constructor
    Player(QObject *parent = 0);
    //Destructor
    ~Player();
    //Load a video from memory
    bool loadVideo(string filename);
    //Play the video
    void Play();
    //Stop the video
    void Stop();
    //check if the player has been stopped
    bool isStopped() const;
    void setCannyThreshold(int t);

     void makeSobelX(const Mat& img, Mat& dst);
     void makeSobelY(const Mat& img, Mat& dst);
     vector<Line> getLines(const Mat& src, Rect roi, bool isVertical);
     vector<Point> cornerDetect(const Mat& src, const Rect& roi);
     void toSW(Mat& img, int threshold);
     double detectDirection(const Mat& frame);
};

#endif // PLAYER_H
