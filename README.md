# CSE-2431_Kernel-Module

# About this project
The GUI portion of this repository uses the GTK framework; in order to compile the GUI code, you must have the GTK library installed on your computer. Most linux distros have this installed by default, but if your distro does not, you should run this line in bash: 

>sudo apt-get install gtk2.0

Followed by this line, to install the dev package:

>sudo apt-get install build-essential libgtk2.0-dev

Now, the user should be able to successfully use the makefile to build the project.

 # How to use
  ## Setup 
  After cloning the repository to your machine, you will first need to use the 
  
  >make
  
  command in order to compile the source code into binaries. The makefile should automatically determine your kernel version and        construct a kernel module approrpriate for your version. 
  Once you have obtained the binaries, run the following in a superuser enabled prompt:
  
  > ./module.load
  
  Now the kernel module should be loaded into the system and ready for use.
  There are two provided methods for interacting with the module, and instructions for each will be given below. 
  
  ## GUI
  To launch the GUI, invoke the following command in a superuser enabled prompt
  
  >./time_modifier_gui.c
  
  in the repo directory. This should launch a new window that bears resemblance to a "properties" window for a file. 
  In order to use this window, first use the file selector to select the desired file. Upon selecting the file, the time displays
  should be populated. Now you can use the edit boxes beneath them to . To commit your desired changes, click the "OK" or "Apply" 
  buttons. Note that the desired format for the new date is DD:MM:YYYY-HH.MM.SS.MS, and the program will reject invalid input.
  
  ## Command line program
  To launch the command line, invoke the following command in a superuser enabled prompt:
  
  >[REPO_PATH]/time_modifier_command_line.c [TARGET_FILE_PATH] [TYPE_TO_CHANGE] [NEW_TIME]
  
  Here, [TARGET_FILE_PATH] is a fully-specified path to the file that the user wants to change, 
  [TYPE_TO_CHANGE] is the trait to modify (where LM is last modified, DC is date changed, and DA is date accessed), and
  [NEW_TIME] is the new time to change the property to (expects DD:MM:YYYY-HH.MM.SS.MS format).
  
  For example, if the user wanted to change the Last Modified property of a file named test_file in their documents folder
  to 11/30/2018 then they could issue this command
  
  ./time_modifier_command_line.c /home/user/Documents/test_file LM 30.11.2018-00:00:00:00

  The command line input will be validated, and then it will programatically invoke the kernel module to change the user specified 
  action.

