#ifndef LINE_H
#define LINE_H

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <iostream>

using namespace std;

class Line
{
    cv::Point2d pRight, pLeft;

public:
    Line(double r, double omega);
    double m, y0, omega;
    cv::Vec2d r;

    void draw(cv::Mat& src, cv::Scalar color);
};

#endif // LINE_H
