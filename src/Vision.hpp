#pragma once

#include "cluon-complete.hpp"
// #include "opendlv-standard-message-set.hpp"
// #include <opencv2/highgui/highgui.hpp>
// #include <opencv2/imgproc/imgproc.hpp>

class Vision {
    public:
    cv::Scalar filterLower;
    cv::Scalar filterUpper;
    Vision();
    void filterImage(cv::Mat img);    
}