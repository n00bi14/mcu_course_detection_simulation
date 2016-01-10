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

    double d = 100;
    double angle = detectDirection(frame);
    double l = sin(angle) / d;
    double m = sqrt(d*d - l*l);
    line(frame, Point(cvRound(frame.cols * 0.5), frame.rows), Point(cvRound(frame.cols * 0.5) + l, cvRound(frame.rows - m)), Scalar(0, 200, 200), 3);
}

void Player::run()
{
    int delay = (1000/frameRate);
//    getFrame(videoStart,capture,frame);
    capture >> frame;
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

void Player::getFrame(double second,VideoCapture& cap, Mat& frame){
    cap.set(CV_CAP_PROP_POS_FRAMES, int(second * cap.get(CV_CAP_PROP_FPS)));
    cap >> frame;
}

bool Player::lineInRegionOfIntrest(Rect& roi, Vec4i& line){
    Point p1(line[0], line[1]);
    Point p2(line[2], line[3]);
}

//-- Computer vision shit:

void Player::makeSobelX(const Mat& img, Mat& dst){

    Mat afterSobel, afterGaussian;
    afterSobel = afterGaussian = img.clone();

    int kernelSize = 3;
    double sigmaX = (kernelSize / 6) + 1;
    GaussianBlur(img,dst, Size(kernelSize,kernelSize), sigmaX);

    Sobel(dst,dst,CV_64F,1,0,kernelSize);
    convertScaleAbs( dst, dst );

}

void Player::makeSobelY(const Mat& img, Mat& dst){

    Mat afterSobel, afterGaussian;
    afterSobel = afterGaussian = img.clone();

    int kernelSize = 3;
    double sigmaX = (kernelSize / 6) + 1;
    GaussianBlur(img,dst, Size(kernelSize,kernelSize), sigmaX);

    Sobel(dst,dst,CV_64F,0,1,kernelSize);
    convertScaleAbs( dst, dst );

}

vector<Line> Player::getLines(const Mat& src, Rect roi, bool isVertical) {
    if(src.channels() != 1)
        assert("fuck off with your many channels!");

    Mat dst;

    if(isVertical){
        makeSobelX(src, dst);
    }else{
        makeSobelY(src, dst);
    }
    Canny(dst, dst, 80, 80, 3);

    Mat image_roi_center = Mat(dst, Range(roi.y, roi.y+roi.height), Range(roi.x, roi.x+roi.width));

    vector<Vec2f> lines;
    HoughLines(image_roi_center, lines, 1, CV_PI/180, 50);//, 50, 10 );

    vector<Line> ownLine;
    for(unsigned int i = 0; i < lines.size(); i++){
        Vec2f line = lines[i];
        Line l(line[0], line[1]);
        ownLine.push_back(l);
    }

    return ownLine;
}

vector<Point> Player::cornerDetect(const Mat& src, const Rect& roi){

    static int i = 0;

    Mat image_roi_center = Mat(src, Range(roi.y, roi.y+roi.height), Range(roi.x, roi.x+roi.width));
    Mat kernel(5, 5, CV_8UC1);

    makeSobelX(image_roi_center, image_roi_center);

    Mat dst, dst_norm;
    vector<Point> points = vector<Point>();

    int thresh = 150;
    int blockSize = 2;
    int apertureSize = 3;
    double k = 0.04;

    vector<cv::Vec4i> lines;
    HoughLinesP(image_roi_center,lines, 1, CV_PI/180, 50);
//    for(unsigned int i = 0; i < lines.size(); i++){
//        line(image_roi_center, Point(lines[i][0], lines[i][1]), Point(lines[i][2], lines[i][3]), Scalar(255,255,255), 6);
//    }

    erode(image_roi_center, image_roi_center, kernel);

    /// Detecting corners
    cornerHarris( image_roi_center, dst, blockSize, apertureSize, k, BORDER_DEFAULT );

    /// Normalizing
    normalize( dst, dst_norm, 0, 255, NORM_MINMAX, CV_32FC1, Mat() );

    /// Drawing a circle around corners
    for( int j = 0; j < dst_norm.rows ; j++ ){
        for( int i = 0; i < dst_norm.cols; i++ ){
            if( (int) dst_norm.at<float>(j,i) > thresh ){
                points.push_back(Point( i, j ));
            }
        }
    }

    return points;
}

void Player::toSW(Mat& img, int threshold){
    for(int i = 0; i < img.rows; i++){
        for(int j = 0; j < img.cols; j++){
            uchar& currVal = img.at<uchar>(i,j);
            currVal = ( currVal < threshold ) ? 0 : 255;
        }
    }
}


double Player::detectDirection(const Mat& frame) {

    Rect roi_center(500, 0, 300, frame.rows-300);
    Rect roi_left(50, 0, 350, frame.rows-300);
    Rect roi_right(900, 0, 350, frame.rows-300);

    Mat dst, cdst;
    if(frame.channels() == 3)
        cvtColor(frame, dst, CV_BGR2GRAY);

    Canny(dst, dst, 80, 80, 3);

    vector<Line> ownLines = getLines(frame, roi_center, true);
    // TODO: decide steering angle, to change lane or drive 90° curve
    //steering angle in curves < 90° and on straight lane
    double angle = 0;
    for(size_t i = 0; i < ownLines.size(); i++)
        angle += (ownLines[i].omega - M_PIl);

    angle /= ownLines.size();

    vector<Point> corner_points = cornerDetect(dst, roi_center);

    return angle;
}
