# CSE-2431_Kernel-Module

# About this project
The GUI portion of this repository uses the GTK framework; in order to compile the GUI code, you must have the GTK library installed on your computer. Most linux distros have this installed by default, but if your distro does not, you should run this line in bash. 

>sudo apt-get install gtk2.0

Followed by this line, to install the dev package.

>sudo apt-get install build-essential libgtk2.0-dev

Now, the user should be able to successfully use the makefile to build the project.

 # How to use
  ## Setup 
  After cloning the repository to your machine, you will first need to use the 
  
  >make
  
  command in order to compile the source code into binaries. The makefile should automatically determine your kernel version and        construct a kernel module approrpriate for your version. 
  Once you have obtained the binaries, run
  
  > ./module.load
  
  in a superuser enabled prompt. Now the kernel module should be loaded into the system and ready for use.
  There are two provided methods for interacting with the module, and instructions for each will be given below. 
  
  ## GUI
  To launch the GUI, invoke the following command in a superuser enabled prompt
  >./time_modifier_gui.c
  in the repo directory. This should launch a new window that bears resemblance to a "properties" window for a 
  ## Command line program
  

