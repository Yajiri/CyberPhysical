/*
 * Copyright (C) 2020  Christian Berger
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// Include the single-file, header-only middleware libcluon to create high-performance microservices
#include "cluon-complete.hpp"
// Include the OpenDLV Standard Message Set that contains messages that are usually exchanged for automotive or robotic applications 
// #include "opendlv-standard-message-set.hpp"
#include "opendlv-standard-message-set.hpp"

// Include the GUI and image processing header files from OpenCV
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <string>
#include <memory>
#include <stdexcept>

// TODO: clean up code structure with header files
// TODO: resolve compilation warnings

// Declaring constants
std::tuple<cv::Scalar, cv::Scalar> YELLOW_FILTER = {cv::Scalar(15, 62, 139), cv::Scalar(40, 255, 255)};
std::tuple<cv::Scalar, cv::Scalar> BLUE_FILTER = {cv::Scalar(110, 91, 45), cv::Scalar(134, 194, 96)};

double CONTOUR_AREA_THRESHOLD = 5;
double ERROR_GROUND_ZERO = 0.05; // The allowed absolute deviation if the ground angle is zero
double ERROR_MULTI = 0.3; // The allowed relative deviation if the angle is not zero

//Function declaration
cv::Point findCenter(cv::Rect rectangle);
cv::Mat drawCenter(cv::Mat sourceImg, std::vector<cv::Rect> cones);
void calculateDistance(cv::Point p1, cv::Point p2);

// Filters and image out-place according to HSV bounds and returns it.
cv::Mat filterImage(cv::Mat sourceImage, std::tuple<cv::Scalar, cv::Scalar> filter) {
    cv::Mat imgHSV, filteredImage, mask;
    cv::cvtColor(sourceImage, imgHSV, cv::COLOR_BGR2HSV);
    cv::inRange(imgHSV, std::get<0>(filter), std::get<1>(filter), mask);
    sourceImage.copyTo(filteredImage, mask);
    return filteredImage;
}

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

int32_t main(int32_t argc, char **argv) {
    int32_t retCode{1};
    int totalFrames = 0, correctFrames = 0;
    float leftVoltage, rightVoltage;
    cv::Point coneCenter;
    cv::Mat conesWithCenter;
    
    // Parse the command line parameters as we require the user to specify some mandatory information on startup.
    auto commandlineArguments = cluon::getCommandlineArguments(argc, argv);
    if ( (0 == commandlineArguments.count("cid")) ||
         (0 == commandlineArguments.count("name")) ||
         (0 == commandlineArguments.count("width")) ||
         (0 == commandlineArguments.count("height")) ) {
        std::cerr << argv[0] << " attaches to a shared memory area containing an ARGB image." << std::endl;
        std::cerr << "Usage:   " << argv[0] << " --cid=<OD4 session> --name=<name of shared memory area> [--verbose]" << std::endl;
        std::cerr << "         --cid:    CID of the OD4Session to send and receive messages" << std::endl;
        std::cerr << "         --name:   name of the shared memory area to attach" << std::endl;
        std::cerr << "         --width:  width of the frame" << std::endl;
        std::cerr << "         --height: height of the frame" << std::endl;
        std::cerr << "Example: " << argv[0] << " --cid=253 --name=img --width=640 --height=480 --verbose" << std::endl;
    }
    else {
        // Extract the values from the command line parameters
        const std::string NAME{commandlineArguments["name"]};
        const uint32_t WIDTH{static_cast<uint32_t>(std::stoi(commandlineArguments["width"]))};
        const uint32_t HEIGHT{static_cast<uint32_t>(std::stoi(commandlineArguments["height"]))};
        const bool VERBOSE{commandlineArguments.count("verbose") != 0};

        // Attach to the shared memory.
        std::unique_ptr<cluon::SharedMemory> sharedMemory{new cluon::SharedMemory{NAME}};
        if (sharedMemory && sharedMemory->valid()) {
            std::clog << argv[0] << ": Attached to shared memory '" << sharedMemory->name() << " (" << sharedMemory->size() << " bytes)." << std::endl;

            // Interface to a running OpenDaVINCI session where network messages are exchanged.
            // The instance od4 allows you to send and receive messages.
            cluon::OD4Session od4{static_cast<uint16_t>(std::stoi(commandlineArguments["cid"]))};

            opendlv::proxy::GroundSteeringRequest gsr;
            opendlv::proxy::VoltageReading vr;
            std::mutex gsrMutex, vrMutex;

            auto onGroundSteeringRequest = [&gsr, &gsrMutex](cluon::data::Envelope &&env){
                // The envelope data structure provide further details, such as sampleTimePoint as shown in this test case:
                // https://github.com/chrberger/libcluon/blob/master/libcluon/testsuites/TestEnvelopeConverter.cpp#L31-L40
                std::lock_guard<std::mutex> lck(gsrMutex);
                gsr = cluon::extractMessage<opendlv::proxy::GroundSteeringRequest>(std::move(env));
            };

            auto onVoltageReading = [&vr, &vrMutex, &leftVoltage, &rightVoltage](cluon::data::Envelope &&env) {
                std::lock_guard<std::mutex> lck(vrMutex);
                vr = cluon::extractMessage<opendlv::proxy::VoltageReading>(std::move(env));
                int senderStamp = env.senderStamp();
                if(senderStamp == 1) leftVoltage = vr.voltage();
                if(senderStamp == 3) rightVoltage = vr.voltage();
            };

            od4.dataTrigger(opendlv::proxy::GroundSteeringRequest::ID(), onGroundSteeringRequest);
            od4.dataTrigger(opendlv::proxy::VoltageReading::ID(), onVoltageReading);

            // Endless loop; end the program by pressing Ctrl-C.
            while (od4.isRunning()) {
                // OpenCV data structure to hold an image.
                cv::Mat img;

                // Wait for a notification of a new frame.
                sharedMemory->wait();

                // Lock the shared memory.
                sharedMemory->lock();
                {
                    // Copy the pixels from the shared memory into our own data structure.
                    cv::Mat wrapped(HEIGHT, WIDTH, CV_8UC4, sharedMemory->data());
                    img = wrapped.clone();
                }
                sharedMemory->unlock();

                float groundSteering;
                {
                    std::lock_guard<std::mutex> lck(gsrMutex);
                    groundSteering = gsr.groundSteering();
                }
                
                // Detecting both color cones [bounding rectangles]
                std::vector<cv::Rect> yellowCones = detectCones(filterImage(img, YELLOW_FILTER));
                std::vector<cv::Rect> blueCones = detectCones(filterImage(img, BLUE_FILTER));

                // Testing metrics
                float calculatedSteering = calculateAngle(leftVoltage, rightVoltage);
                float dGroundSteering = groundSteering == 0 ? ERROR_GROUND_ZERO : groundSteering * ERROR_MULTI;
                bool calculatedWithinInterval = fabs(groundSteering - calculatedSteering) < dGroundSteering;

                // Blacking out the horizon and wires of the car
                cv::rectangle(img, cv::Point(0, 0), cv::Point(640, 0.45 * 480), cv::Scalar(0, 0, 0), cv::FILLED);
                cv::rectangle(img, cv::Point(160, 390), cv::Point(495, 479), cv::Scalar(0, 0, 0), cv::FILLED);

                // Drawing bounding rectangles over detected cones
                for (auto &cone : joinVectors(yellowCones, blueCones))
                {
                    cv::rectangle(img, cv::Point(cone.x, cone.y), cv::Point(cone.x + cone.width, cone.y + cone.height), cv::Scalar(0, 0, 255), 2);
                }

                // Display image on your screen.
                if (VERBOSE) {
                    std::cout << "----------- FRAME REPORT -----------" << std::endl;
                    std::cout << "[LEFT VOLTAGE] Got " << leftVoltage << std::endl;
                    std::cout << "[RIGHT VOLTAGE] Got " << rightVoltage << std::endl;
                    std::cout << "[GROUND STEERING] Got " << groundSteering << ". Allowed values [" << groundSteering - dGroundSteering << "," << groundSteering + dGroundSteering << "]" << std::endl;
                    std::cout << "[CALCULATED STEERING] Got " << calculatedSteering << ". " << (calculatedWithinInterval ? "[SUCCESS]" : "[FAILURE]") << std::endl;
                    totalFrames++;
                    correctFrames += calculatedWithinInterval ? 1 : 0;
                    std::cout << "[RESULT] Correctly calculated " << (float)(100*correctFrames) / (float)totalFrames << "\% frames" << std::endl;
                    std::cout << "LEFT = " << leftVoltage << "; RIGHT = " << rightVoltage << ";" << std::endl;
                    // std::cout << "----------- CONES DETECTION -----------" << std::endl;
                    
                    // If cones are detected, draw a point in the center of each rectangle
                    if(yellowCones.size() > 0) {
                    //   std::cout << "Yellow cones: " << std::endl;  
                    //   img = drawCenter(img,yellowCones);
                    }
                     if(blueCones.size() > 0) {
                    //   std::cout << "Blue cones: " << std::endl;   
                    //   img = drawCenter(img,blueCones);
                    }
        		
                    cv::imshow(sharedMemory->name().c_str(), img);

                    cv::waitKey(1);

                    // Logging detected cone locations and sizes
                    int blueConeIndex = 1;
                    int yellowConeIndex = 1;
                    for (auto& cone : yellowCones) {
                        // std::cout << "Detected yellow cone #" << yellowConeIndex << ": ";
                        // std::cout << "x = " << cone.x << "; y = " << cone.y << "; width = " << cone.width << "; height = " << cone.height << ";" << std::endl;
                        yellowConeIndex++;
                    }

                    for (auto& cone : blueCones) {
                        // std::cout << "Detected blue cone #" << blueConeIndex << ": ";
                        // std::cout << "x = " << cone.x << "; y = " << cone.y << "; width = " << cone.width << "; height = " << cone.height << ";" << std::endl;
                        blueConeIndex++;
                    }
                }
            }
        }
        retCode = 0;
    }
    return retCode;
}

// Function definition
// Find center of one rectangle
cv::Point findCenter(cv::Rect rectangle) {
    cv::Point center;
    center.x = rectangle.width / 2 + rectangle.x;
    center.y = rectangle.height / 2 + rectangle.y;
    return center;
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

void calculateDistance(cv::Point p1, cv::Point p2) {
    std::cout << "Distance: " << std::sqrt(std::pow(p2.x - p1.x, 2) + std::pow(p2.y - p1.y, 2)) << std::endl; 
}





