#include "Vision.hpp";
#include "cluon-complete.hpp";

Vision::Vision() {};

// Lower and upper bounds for HSV filtering.
Vision::filterLower = cv::Scalar(15, 62, 139);
Vision::filterUpper = cv::Scalar(40, 255, 255);

// Filters and image out-place according to HSV bounds and returns it.
cv::Mat Vision::filterImage(cv::Mat sourceImage) {
    cv::Mat imgHSV, filteredImage, mask;
    cv::cvtColor(sourceImage, imgHSV, cv::COLOR_BGR2HSV);
    cv::inRange(imgHSV, lower, upper, mask);
    sourceImage.copyTo(filteredImage, mask);
    return filteredImage;
}