/****************************************************************
 *
 * Copyright (c) 2010
 *
 * Fraunhofer Institute for Manufacturing Engineering	
 * and Automation (IPA)
 *
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Project name: care-o-bot
 * ROS stack name: cob3_driver
 * ROS package name: cob3_cameraSensors
 *								
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *			
 * Author: Florian Weisshardt, email:florian.weisshardt@ipa.fhg.de
 * Supervised by: Jan Fischer, email:jan.fischer@ipa.fhg.de
 *
 * Date of creation: Jan 2010
 * ToDo:
 *
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Fraunhofer Institute for Manufacturing 
 *       Engineering and Automation (IPA) nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License LGPL as 
 * published by the Free Software Foundation, either version 3 of the 
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License LGPL for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public 
 * License LGPL along with this program. 
 * If not, see <http://www.gnu.org/licenses/>.
 *
 ****************************************************************/

//##################
//#### includes ####

// standard includes
//--

// ROS includes
#include <ros/ros.h>

// ROS message includes
#include <sensor_msgs/Image.h>
#include <sensor_msgs/fill_image.h>

// ROS service includes
#include <cob3_srvs/Init.h>
#include <cob3_srvs/Stop.h>
#include <cob3_srvs/GetCameraInfo.h>

// external includes
#include <AbstractColorCamera.h>
#include <GlobalDefines.h>

using namespace ipa_CameraSensors;

//##########################
//#### global variables ####
//--

//##################################
//#### topic callback functions ####
// function will be called when a new message arrives on a topic
//--

//####################################
//#### service callback functions ####
// function will be called when a service is querried

bool srvCallback_Init(cob3_srvs::Init::Request &req,
                      cob3_srvs::Init::Response &res )
{
    ROS_INFO("Initializing camera");
    res.success = 0; // 0 = true, else = false
    return true;
}

bool srvCallback_Stop(cob3_srvs::Stop::Request &req,
                      cob3_srvs::Stop::Response &res )
{
    ROS_INFO("Stopping camera");
    res.success = 0; // 0 = true, else = false
    return true;
}

bool srvCallback_GetCameraInfo(cob3_srvs::GetCameraInfo::Request &req,
                               cob3_srvs::GetCameraInfo::Response &res )
{
    ROS_INFO("get camera info");
    sensor_msgs::CameraInfo cameraInfo;
    //TODO: get real camera Info
    res.cameraInfo = cameraInfo;
    res.success = 0; // 0 = true, else = false
    return true;
}

//#######################
//#### main programm ####
int main(int argc, char** argv)
{
	// initialize ROS, spezify name of node
	ros::init(argc, argv, "cob3_driver_colorCamera_left");

	// create a handle for this node, initialize node
	ros::NodeHandle n;
	
    // topics to publish
    ros::Publisher topicPub_Image = n.advertise<sensor_msgs::Image>("cob3/colorCamera/left/Image", 1);
    
	// topics to subscribe, callback is called for new messages arriving
    //--
    
    // service servers
    ros::ServiceServer srvServer_Stop = n.advertiseService("cob3/colorCamera/left/Stop", srvCallback_Stop);
    ros::ServiceServer srvServer_Init = n.advertiseService("cob3/colorCamera/left/Init", srvCallback_Init);
    ros::ServiceServer srvServer_GetCameraInfo = n.advertiseService("cob3/colorCamera/left/GetCameraInfo", srvCallback_GetCameraInfo);
        
    // service clients
    //--
    
    // external code

	/// Camera index ranges from 0 (right) to 1 (left)
	int cameraIndex = 1;
	IplImage* image = 0;
	std::string directory = "../files/";

  	AbstractColorCamera* colorCamera = 0;
	colorCamera = ipa_CameraSensors::CreateColorCamera_AVTPikeCam();

	if (colorCamera->Init(directory, cameraIndex) & ipa_CameraSensors::RET_FAILED)
    {
            std::cerr << "ERROR - CameraDataViewerControlFlow::Init:" << std::endl;
            std::cerr << "\t ... Error while initializing color sensor 0.\n";
            colorCamera = 0;
    }

    if (colorCamera && (colorCamera->Open() & ipa_CameraSensors::RET_FAILED))
    {
            std::cerr << "ERROR - CameraDataViewerControlFlow::Init:" << std::endl;
            std::cerr << "\t ... Error while opening color sensor 0.\n";
            colorCamera = 0;
    }

	// display image
	//cvNamedWindow("Image", CV_WINDOW_AUTOSIZE);

    // main loop
    ros::Rate loop_rate(3); // Hz
	while (n.ok())
	{
		if (image != 0)
		{
			cvReleaseImage(&image);
			image = 0;
		}

        if (colorCamera->GetColorImage2(&image) & ipa_Utils::RET_FAILED)
        {
                std::cerr << "ERROR - CameraDataViewerControlFlow::ShowSharedImage:" << std::endl;
                std::cerr << "\t ... Color image acquisition from camera 0 failed." << std::endl;
                return ipa_Utils::RET_FAILED;
        }

        // create message
        sensor_msgs::Image msg;
		fillImage(msg, "bgr8", image->height, image->width, image->widthStep, image->imageData);
		msg.header.stamp = ros::Time::now();    
  
        // publish message
        ROS_INFO("published image from colorCamera_left");
		
		// uncomment to display image
		//cvShowImage("Image", image);
		//cvWaitKey(10);

        topicPub_Image.publish(msg);

        // sleep and waiting for messages, callbacks    
        ros::spinOnce();
        loop_rate.sleep();
    }

	colorCamera->Close();
	ipa_CameraSensors::ReleaseColorCamera(colorCamera);
    
    return 0;
}
