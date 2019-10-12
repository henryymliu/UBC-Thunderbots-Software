#!/bin/bash

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#
# UBC Thunderbots Archlinux Software Setup
#
# This script must be run with sudo! root permissions are required to install
# packages and copy files to the /etc/udev/rules.d directory. The reason that the script
# must be run with sudo rather than the individual commands using sudo, is that
# when running CI within Docker, the sudo command does not exist since
# everything is automatically run as root.
#
# This script will install all the required libraries and dependencies to build
# and run the Thunderbots codebase. This includes being able to run the ai and
# unit tests
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~#


# Save the parent dir of this so we can always run commands relative to the
# location of this script, no matter where it is called from. This
# helps prevent bugs and odd behaviour if this script is run through a symlink
# or from a different directory.
CURR_DIR=$(dirname -- "$(readlink -f -- "$BASH_SOURCE")")
cd $CURR_DIR
# The root directory of the reopsitory and git tree
GIT_ROOT="$CURR_DIR/.."


echo "================================================================"
echo "Installing Utilities and Dependencies"
echo "================================================================"

sudo pacman -Sy

host_software_packages=(
    curl
    cmake
    protobuf-compiler
    libprotobuf-dev
    libusb
    qt5 # The GUI library for our visualizer
    libeigen3-dev # A math / numerical library used for things like linear regression
    python-yaml # yaml for cfg generation (Dynamic Parameters)
)
sudo pacman -S ${host_software_packages[@]} --noconfirm

if [ $? -ne 0 ]; then
    echo "##############################################################"
    echo "Error: Installing utilities and dependencies failed"
    echo "##############################################################"
    exit 1
fi

# Install Bazel
echo "================================================================" 
echo "Installing Bazel"
echo "================================================================"

# Adapted from https://docs.bazel.build/versions/master/install-ubuntu.html#install-on-ubuntu
sudo apt-get update
sudo apt-get install openjdk-11-jdk -y
echo "deb [arch=amd64] https://storage.googleapis.com/bazel-apt stable jdk1.8" | sudo tee /etc/apt/sources.list.d/bazel.list
curl https://bazel.build/bazel-release.pub.gpg | sudo apt-key add -
sudo apt-get update
sudo apt-get install bazel -y
if [ $? -ne 0 ]; then
    echo "##############################################################"
    echo "Error: Installing Bazel failed"
    echo "##############################################################"
    exit 1
fi

# Symlink qt include directory
ln -sf /usr/include/x86_64-linux-gnu/qt5 ../src/external/qt

# Done
echo "================================================================"
echo "Done Software Setup"
echo "================================================================"

