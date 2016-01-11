#include "line.h"

Line::Line(double r, double omega)
{
    double a,b;
    this->omega = omega;
    this->r = cv::Vec2d(r*cos(omega), r*sin(omega));

    a = cos(omega);
    b = -sin(omega);

    pRight.x = this->r[0] + 1000 * b;
    pRight.y = this->r[1] + 1000 * a;

    pLeft.y = this->r[1] - 1000 * a;
    pLeft.x = this->r[0] - 1000 * b;

    this->m = (pRight.y - pLeft.y)/(pRight.x - pLeft.x);
    this->y0 =  pRight.y - m * pRight.x;
}

void Line::draw(cv::Mat& src, cv::Scalar color){

    cv::line(src, cv::Point(cvRound(pRight.x), cvRound(pRight.y)), cv::Point(cvRound(pLeft.x), cvRound(pLeft.y)), color, 1, CV_AA);

}
