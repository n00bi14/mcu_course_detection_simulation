#include "player.h"



Player::Player(QObject *parent)
 : QThread(parent)
{
    stop = true;
    videoStart = 9;
    cannyThreshold = 80;
}

bool Player::loadVideo(string filename) {
    capture.open(filename);
    if (capture.isOpened())
    {
        frameRate = (int) capture.get(CV_CAP_PROP_FPS);
        return true;
    }
    else
        return false;
}

void Player::Play()
{
    if (!isRunning()) {
        if (isStopped()){
            stop = false;
        }
        start(LowPriority);
    }
}

void Player::processFrame(Mat& frame)
{
    if(frame.channels() == 3)
        cvtColor(frame, frame, CV_BGR2GRAY);

//    GaussianBlur(frame, frame, Size(3, 3), 4);
//    Canny(frame, frame, cannyThreshold, cannyThreshold);

    Mat dst, cdst;
//    Canny(frame, dst, 50, 200, 3);
    Canny(frame, dst, cannyThreshold, cannyThreshold, 3);
    cvtColor(dst, cdst, CV_GRAY2BGR);

    vector<Vec4i> lines;
    HoughLinesP(dst, lines, 1, CV_PI/180, 50, 50, 10 );
    for( size_t i = 0; i < lines.size(); i++ )
    {
        Vec4i l = lines[i];
        line( cdst, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, CV_AA);
    }
    cdst.copyTo(frame);
}

void Player::run()
{
    int delay = (1000/frameRate);
    getFrame(videoStart,capture,frame);
    while(!stop){
        if (!capture.read(frame))
        {
            stop = true;
        }

        processFrame(frame);

        if (frame.channels()== 3){
            cv::cvtColor(frame, RGBframe, CV_BGR2RGB);
            img = QImage((const unsigned char*)(RGBframe.data),
                              RGBframe.cols,RGBframe.rows,QImage::Format_RGB888);
        }
        else
        {
            img = QImage((const unsigned char*)(frame.data),
                                 frame.cols,frame.rows,QImage::Format_Indexed8);
        }
        emit processedImage(img);
        this->msleep(delay);
    }
}

Player::~Player()
{
    mutex.lock();
    stop = true;
    capture.release();
    condition.wakeOne();
    mutex.unlock();
    wait();
}
void Player::Stop()
{
    stop = true;
}
void Player::msleep(int ms){
    struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
    nanosleep(&ts, NULL);
}
bool Player::isStopped() const{
    return this->stop;
}

void Player::setCannyThreshold(int t)
{
    mutex.lock();
    this->cannyThreshold = t;
    mutex.unlock();
}

void Player::getFrame(int second,VideoCapture& cap, Mat& frame){
    for(int i = 0; i < second; i++){
        for(int j = 0; j < cap.get(CV_CAP_PROP_FPS); j++){
            cap >> frame;
        }
    }
}
