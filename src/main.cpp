#include <iostream>
#include <opencv2/opencv.hpp>
#include <boost/lexical_cast.hpp>

#include "CmdLineInterface.hpp"
#include "AppConfig.hpp"
#include "Target.hpp"
#include "TargetDetector.hpp"
#include "TargetProcessor.hpp"
#include "NetworkController.hpp"
#include "VideoDevice.hpp"
#include "GUIManager.hpp"

int main(int argc, char* argv[])
{
    // Create all the necessary objects (calls the default constructor for each)
    TargetDetector detector;
    TargetProcessor processor;
    NetworkController networkController;
    VideoDevice camera;
	CmdLineInterface interface(argc, argv);		
	AppConfig config = interface.getConfig();
	GUIManager gui;

	if(config.getIsDevice())
    {
        camera.startCapture(config.getDeviceID());
        if(config.getIsDebug())
            std::cout << "Camera ready!\n";
    }

    //init networking
    if(config.getIsNetworking())
        networkController.startServer();

    if(!config.getIsHeadless())
        gui.init();
    if(config.getIsDebug())
	 std::cout << "Im debugging! :D\n";

    while(true)
    {
        networkController.waitForPing();

        cv::Mat image = camera.getImage();
        Target* target = detector.processImage(image);

        bool foundTarget = (target == NULL ? false : true);

        if(foundTarget)
        {
            processor.loadTarget(target);
            double distance = processor.calculateDistance();

            networkController.sendMessage("true;" +
                boost::lexical_cast<std::string> (distance));

            std::cout << "Target Found! Distance: " << distance;
        }
        else
        {
            networkController.sendMessage("false;");
        }
    }

    return 0;
}
