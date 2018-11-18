#include <ros/ros.h>
#include <ros/time.h>
#include <thunderbots_msgs/Primitive.h>
#include <thunderbots_msgs/PrimitiveArray.h>
#include <gtkmm/main.h>

#include <iostream>
#include <thread>
#include <random>

#include "ai/primitive/move_primitive.h"
#include "ai/primitive/primitive.h"
#include "backend_output/grsim/grsim_backend.h"
#include "backend_output/mrf/mrf_backend.h"
#include "geom/point.h"
#include "util/constants.h"

// Constants
const std::string NETWORK_ADDRESS       = "127.0.0.1";
static constexpr short NETWORK_PORT     = 20011;
static constexpr unsigned int TICK_RATE = 30;

// Member variables we need to maintain state

// A vector of primitives. It is cleared each tick, populated by the callbacks
// that receive primitive commands, and is processed by the backend to send
// the primitives to the system we have chosen (such as grSim, our radio, etc.)
std::vector<std::unique_ptr<Primitive>> primitives;

// Callbacks
void primitiveUpdateCallback(const thunderbots_msgs::PrimitiveArray::ConstPtr& msg)
{
    thunderbots_msgs::PrimitiveArray prim_array_msg = *msg;
    for (const thunderbots_msgs::Primitive& prim_msg : prim_array_msg.primitives)
    {
        primitives.emplace_back(Primitive::createPrimitive(prim_msg));
    }
}

int main(int argc, char** argv)
{
    // Init ROS node
    ros::init(argc, argv, "backend_output");
    ros::NodeHandle node_handle;

    // Hack: need to make this a GTK app to get the libusb signal stuff working :(
    Gtk::Main app(argc, argv);

    // Create subscribers to topics we care about
    ros::Subscriber prim_array_sub = node_handle.subscribe(
        UTIL::CONSTANTS::AI_PRIMITIVES_TOPIC, 1, primitiveUpdateCallback);

    // Initialize variables
    primitives           = std::vector<std::unique_ptr<Primitive>>();
    // GrSimBackend backend = GrSimBackend(NETWORK_ADDRESS, NETWORK_PORT);
    MRFDongle dongle = MRFDongle();
    MrfBackend backend = MrfBackend(dongle);
    // We loop at 30Hz so we don't overload the network with too many packets
    ros::Rate tick_rate(TICK_RATE);
    
    // Let the GTK main loop run in its own thread.
    std::thread t{[&app]{app.run();}};

    // Main loop
    while (ros::ok())
    {
        // Clear all primitives each tick
        primitives.clear();
        primitives.emplace_back(std::make_unique<MovePrimitive>(6, 
            Point(std::rand()%4, std::rand()%4), Angle::ofDegrees(std::rand()%360), 3));

        // Spin once to let all necessary callbacks run
        // The callbacks will populate the primitives vector
        ros::spinOnce();

        backend.sendPrimitives(primitives);

        tick_rate.sleep();
    }
    t.join();

    return 0;
}
