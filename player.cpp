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
        roi_center = Rect(0, 0, 0, 0);
        roi_left = Rect(0, 0, 0, 0);
        roi_right = Rect(0, 0, 0, 0);

        goTo(8.);

        frameRate = (int) capture.get(CV_CAP_PROP_FPS);
        videoLen = double(capture.get(CV_CAP_PROP_FRAME_COUNT)) / frameRate;

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
    static double lastAngle = -.5 * M_PIl;

    if(roi_center.width < 1) {
        int height = frame.rows * .6;
        int width = frame.cols * .25;
        roi_center = Rect((frame.cols - width) * .5, 0, width, height);
        roi_left = Rect(50, 0, 350, frame.rows * .6);
        roi_right = Rect(900, 0, 350, frame.rows * .6);
    }

    double d = 100;
    vector<Line> lines;
    double angle = detectDirection(frame, lines);
    if(angle == angle)
        lastAngle = angle;
    double dx = cos(lastAngle) * d;
    double m = sqrt(d*d - dx*dx);
    line(frame, Point(cvRound(frame.cols * 0.5), frame.rows), Point(cvRound(frame.cols * 0.5) + dx, cvRound(frame.rows - m)), Scalar(0, 200, 200), 3);

    circle(frame, Point(cvRound(frame.cols * 0.5) + dx, cvRound(frame.rows - m)), 3, Scalar(255, 0, 255));

    rectangle(frame, roi_center, Scalar(0, 255, 0), 3);

    putText(frame, QString::number(lastAngle).toStdString(), cvPoint(30,30),
        FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(200,200,250), 1, CV_AA);

}

void Player::run()
{
    int delay = (1000/frameRate);
//    getFrame(videoStart,capture,frame);
    capture >> frame;
    while(!stop){
        videoMutex.lock();
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
        videoMutex.unlock();
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

void Player::getFrame(double second, VideoCapture& cap, Mat& frame){
    cap.set(CV_CAP_PROP_POS_FRAMES, int(second * cap.get(CV_CAP_PROP_FPS)));
    cap >> frame;
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

vector<Line> Player::getLines( Mat& src, Rect roi, bool isVertical) {
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
    HoughLines(image_roi_center, lines, 1, CV_PI/180, 70);//, 50, 10 );

    vector<Line> ownLine;
    for(unsigned int i = 0; i < lines.size(); i++){
        Vec2f line = lines[i];
        Line l(line[0], line[1]);
        ownLine.push_back(l);
        //l.draw(image_roi_center_src);
    }

    return ownLine;
}

vector<Point> Player::cornerDetect(const Mat& src, const Rect& roi){

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

    erode(image_roi_center, image_roi_center, kernel);

    //-- Detecting corners
    cornerHarris( image_roi_center, dst, blockSize, apertureSize, k, BORDER_DEFAULT );

    //-- Normalizing
    normalize( dst, dst_norm, 0, 255, NORM_MINMAX, CV_32FC1, Mat() );

    //-- Drawing a circle around corners
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


double Player::detectDirection( Mat& frame, vector<Line>& ownLines) {

    Mat dst, cdst;
    if(frame.channels() == 3)
        cvtColor(frame, dst, CV_BGR2GRAY);

    toSW(dst, 180);

    Canny(dst, dst, 80, 80, 3);

    Mat image_roi_center = Mat(frame, Range(roi_center.y, roi_center.y+roi_center.height), Range(roi_center.x, roi_center.x+roi_center.width));

    double xm = roi_center.width * .5;
    double ym = roi_center.height;

    ownLines = getLines(dst, roi_center, true);
    // TODO: decide steering angle, to change lane or drive 90° curve
    //steering angle in curves < 90° and on straight lane
    double angle = 0;
    int cnt = 0;
    double avgX = 0.;
    double avgY = 0.;
    vector<Point2d> intersecs;
    for(size_t i = 0; i < ownLines.size(); i++) {
        Line l0 = ownLines[i];
        for(size_t n = 0; n < ownLines.size(); n++) {
            Line l1 = ownLines[n];
            //-- choose to different lines ...
            if(n != i && l1.m != l0.m && l1.m * l0.m < 0) {
               //-- ... and calculate their intersection point
               double x = (l0.y0 - l1.y0) / (l1.m - l0.m);
               double y = l1.m * x + l1.y0;
               intersecs.push_back(Point2d(x, y));

               //-- sum up the coord. components to later calculate a kind of average intersection
               //-- (caution: this could be a arbitrary stupid idea)
               avgX += x;
               avgY += y;

               //obsulete
               double omega = atan((y - ym) / (x - xm));
               angle += omega;

               cnt++;
               cv::line(image_roi_center, cv::Point(cvRound(xm), cvRound(ym)), cv::Point(cvRound(x), cvRound(y)), cv::Scalar(255, 0, 0), 1, CV_AA);

            }
        }

        ownLines[i].draw(image_roi_center, cv::Scalar(0, 0, 255));
    }

    if(!angle && ownLines.size() > 0) {
        angle = (ownLines[0].omega > M_PIl) ? -2 * M_PIl + ownLines[0].omega : ownLines[0].omega;
        ownLines[0].draw(image_roi_center, cv::Scalar(255, 0, 0));

    } else {
        avgX /= cnt;
        avgY /= cnt;

        angle = atan((ym - avgY) / (xm - avgX));

        cv::line(image_roi_center, cv::Point(cvRound(xm), cvRound(ym)), cv::Point(cvRound(avgX), cvRound(avgY)), cv::Scalar(0, 255, 255), 1, CV_AA);


    }

    return angle;
}

int Player::getFrameCount()
{
    return capture.get(CV_CAP_PROP_FRAME_COUNT);
}

double Player::getTime()
{

}

void Player::goTo(double frame)
{
    videoMutex.lock();
    capture.set(CV_CAP_PROP_POS_FRAMES, frame);
    videoMutex.unlock();
}
