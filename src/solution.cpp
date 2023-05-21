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
#include <iostream>
#include <fstream>


int32_t main(int32_t argc, char **argv) {
    int32_t retCode{1};
    int totalFrames = 0, correctFrames = 0;
    
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
            opendlv::proxy::AngularVelocityReading avr;
            std::mutex gsrMutex, vrMutex, avrMutex;

            auto onGroundSteeringRequest = [&gsr, &gsrMutex](cluon::data::Envelope &&env){
                // The envelope data structure provide further details, such as sampleTimePoint as shown in this test case:
                // https://github.com/chrberger/libcluon/blob/master/libcluon/testsuites/TestEnvelopeConverter.cpp#L31-L40
                std::lock_guard<std::mutex> lck(gsrMutex);
                gsr = cluon::extractMessage<opendlv::proxy::GroundSteeringRequest>(std::move(env));
            };

            auto onAngularVelocityReading = [&avr, &avrMutex](cluon::data::Envelope &&env) {
                std::lock_guard<std::mutex> lck(avrMutex);
                avr = cluon::extractMessage<opendlv::proxy::AngularVelocityReading>(std::move(env));
            };

            
            od4.dataTrigger(opendlv::proxy::GroundSteeringRequest::ID(), onGroundSteeringRequest);
            od4.dataTrigger(opendlv::proxy::AngularVelocityReading::ID(), onAngularVelocityReading);


            // Endless loop; end the program by pressing Ctrl-C.
            while (od4.isRunning()) {
                // OpenCV data structure to hold an image.
                cv::Mat img;
                double angVelZ = 0.0;
                std::cout << "group_02;";
                // Wait for a notification of a new frame.
                sharedMemory->wait();

                // Lock the shared memory.
                sharedMemory->lock();
                {
                    // Copy the pixels from the shared memory into our own data structure.
                    cv::Mat wrapped(HEIGHT, WIDTH, CV_8UC4, sharedMemory->data());
                    img = wrapped.clone();

                    angVelZ = avr.angularVelocityZ();

                    std::cout << cluon::time::toMicroseconds(sharedMemory->getTimeStamp().second) << ";";

                    
                }
                
                sharedMemory->unlock();

                float groundSteering;
                {
                    std::lock_guard<std::mutex> lck(gsrMutex);
                    groundSteering = gsr.groundSteering();
                
                }

                // Blacking out the horizon and wires of the car
                cv::rectangle(img, cv::Point(0, 0), cv::Point(640, 0.5 * 480), cv::Scalar(0, 0, 0), cv::FILLED);
                cv::rectangle(img, cv::Point(160, 390), cv::Point(495, 479), cv::Scalar(0, 0, 0), cv::FILLED);

                
                float calculatedSteering;                
                
                if(angVelZ <= 0) {
                    if(angVelZ<-78) angVelZ = -78;

                    calculatedSteering = (angVelZ - (-78)) / 78 * 0.3 - 0.3;
                }else if(angVelZ > 0) {
                    if(angVelZ < 2) 
                        angVelZ = 1;
                    calculatedSteering = ((angVelZ - 1)/100)*0.3; 
                }
                std::cout<< calculatedSteering << std::endl;
                

                float dGroundSteering = groundSteering == 0 ? 0.05 : std::abs(0.3 * groundSteering);
                bool calculatedWithinInterval = std::abs(groundSteering - calculatedSteering) <= dGroundSteering;

                // Display image on your screen.
                if (VERBOSE) {
                    // Define the positions for the text
                    cv::Point angularVelocityPos(10, 30);
                    cv::Point frameReportPos(10, 70);

                    // Define the font size
                    double fontSize = 0.6;

                    // Print the angular velocity information
                    std::string angularVelocityText = "Angular Velocity: " + std::to_string(angVelZ);
                    cv::putText(img, angularVelocityText, angularVelocityPos, cv::FONT_HERSHEY_SIMPLEX, fontSize, cv::Scalar(255, 255, 255), 1, cv::LINE_AA);

                    // Print the frame report information
                    cv::putText(img, "----------- FRAME REPORT -----------", frameReportPos, cv::FONT_HERSHEY_SIMPLEX, fontSize, cv::Scalar(255, 255, 255), 1, cv::LINE_AA);
                    cv::putText(img, "[GS] Got " + std::to_string(groundSteering) + ". Allowed [" + std::to_string(groundSteering - dGroundSteering) + "," + std::to_string(groundSteering + dGroundSteering) + "]", cv::Point(frameReportPos.x, frameReportPos.y + 30), cv::FONT_HERSHEY_SIMPLEX, fontSize, cv::Scalar(255, 255, 255), 1, cv::LINE_AA);
                    cv::putText(img, "[CS] Got " + std::to_string(calculatedSteering) + ". " + (calculatedWithinInterval ? "[SUCCESS]" : "[FAILURE]"), cv::Point(frameReportPos.x, frameReportPos.y + 60), cv::FONT_HERSHEY_SIMPLEX, fontSize, cv::Scalar(255, 255, 255), 1, cv::LINE_AA);

                    totalFrames++;
                    correctFrames += calculatedWithinInterval ? 1 : 0;
                    cv::putText(img, "[RESULT] Correctly calculated " + std::to_string((float)(100 * correctFrames) / (float)totalFrames) + "% frames", cv::Point(frameReportPos.x, frameReportPos.y + 90), cv::FONT_HERSHEY_SIMPLEX, fontSize, cv::Scalar(255, 255, 255), 1, cv::LINE_AA);

                    cv::imshow(sharedMemory->name().c_str(), img);
                    cv::waitKey(1);
                }
            }
        }

        retCode = 0;
    }
    return retCode;
}