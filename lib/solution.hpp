#include "cluon-complete.hpp"
#include "opendlv-standard-message-set.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

// Declaring constants
std::tuple<cv::Scalar, cv::Scalar> YELLOW_FILTER = {cv::Scalar(15, 62, 139), cv::Scalar(40, 255, 255)};
std::tuple<cv::Scalar, cv::Scalar> BLUE_FILTER = {cv::Scalar(110, 91, 45), cv::Scalar(134, 194, 96)};
double CONTOUR_AREA_THRESHOLD = 5;
double ERROR_GROUND_ZERO = 0.05; // The allowed absolute deviation if the ground angle is zero
double ERROR_MULTI = 0.3; // The allowed relative deviation if the angle is not zero

// Detects cones by drawing a red rectangle over them and returns the detected cones as an array of Rect
std::vector<cv::Rect> detectCones(cv::Mat sourceImage) {
    cv::Mat grayImage, binaryImage, morphedImage;

    // Convert `sourceImage` to grayscale and store it in `grayImage`
    cv::cvtColor(sourceImage, grayImage, cv::COLOR_BGR2GRAY);

    // Apply threshold 
    cv::threshold(grayImage, binaryImage, 0, 255, cv::THRESH_BINARY);

    // Perform morphological operations
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
    cv::erode(binaryImage, morphedImage, kernel);
    cv::dilate(morphedImage, morphedImage, kernel);

    // Find all contours
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(morphedImage, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    // Filter contours by area
    std::vector<std::vector<cv::Point>> filteredContours;
    for (const auto& contour : contours) {
        double area = cv::contourArea(contour);
        if (area > CONTOUR_AREA_THRESHOLD) {
            filteredContours.push_back(contour);
        }
    }

    std::vector<cv::Rect> boundingRectangles;
    
    // Draw bounding boxes
    for (const auto& contour : filteredContours) {
        cv::Rect boundingRect = cv::boundingRect(contour);
        boundingRectangles.push_back(boundingRect);
        cv::rectangle(sourceImage, boundingRect, cv::Scalar(0, 0, 255), 2);
    }
    
    return boundingRectangles;
}

// Calculates steering angle based on voltage read from left and right IR sensors respectively
float calculateAngle(float leftVoltage, float rightVoltage) {
    // TODO: find the best squish factor
    // TODO: experiment with non-sigmoid squish functions
    // TODO: test 1) playing backwards 2) still frames 3) arbitrary frames 4) all recording files
    float squishFactor = 0.002;
    float leftness = pow(leftVoltage, -1);
    float rightness = pow(rightVoltage, -1);
    float metric = leftness - rightness;
    return 0.6 * pow(1 + pow(2.718, -squishFactor * metric), -1) - 0.3;
}

// Returns a vector concatenation of `first` and `second`
template <typename T>
std::vector<T> joinVectors(std::vector<T> first, std::vector<T> second) {
    std::vector<T> result;
    result.reserve(first.size() + second.size());
    result.insert(result.end(), first.begin(), first.end());
    result.insert(result.end(), second.begin(), second.end());
    return result;
}

// Filters and image out-place according to HSV bounds and returns it.
cv::Mat filterImage(cv::Mat sourceImage, std::tuple<cv::Scalar, cv::Scalar> filter) {
    cv::Mat imgHSV, filteredImage, mask;
    cv::cvtColor(sourceImage, imgHSV, cv::COLOR_BGR2HSV);
    cv::inRange(imgHSV, std::get<0>(filter), std::get<1>(filter), mask);
    sourceImage.copyTo(filteredImage, mask);
    return filteredImage;
}

// Find center of one rectangle
cv::Point findCenter(cv::Rect rectangle) {
    cv::Point center;
    center.x = rectangle.width / 2 + rectangle.x;
    center.y = rectangle.height / 2 + rectangle.y;
    return center;
}

void calculateDistance(cv::Point p1, cv::Point p2) {
    std::cout << "Distance: " << std::sqrt(std::pow(p2.x - p1.x, 2) + std::pow(p2.y - p1.y, 2)) << std::endl; 
}

// Draw the point that represents the center of the given rectangle on the source img 
cv::Mat drawCenter(cv::Mat sourceImg, std::vector<cv::Rect> cones) {
    int carCenterX = (495-160)/2 + 160;
    int carCenterY = (479-390)/2 + 390;

    // center of car 
    cv::circle(sourceImg, cv::Point(carCenterX, carCenterY), 2, cv::Scalar(0, 0, 255), -1);

	int index=1;
	for(auto& cone : cones){
	    cv::Point coneCenter = findCenter(cone);

	    cv::circle(sourceImg, cv::Point(coneCenter.x, coneCenter.y), 2, cv::Scalar(0, 0, 255), -1);

        cv::line(sourceImg, cv::Point(coneCenter.x, coneCenter.y), cv::Point(carCenterX,carCenterY), cv::Scalar(0, 0, 255), 2);

	    std::cout << "Detected center " << index << ": x=" << coneCenter.x << " y=" << coneCenter.y << std::endl;
        
        index++;
        
        calculateDistance(cv::Point(carCenterX,carCenterY), cv::Point(coneCenter.x, coneCenter.y));
	}
	return sourceImg;
}