#Motion Detection analysis repository

Authors: Josip Grlica, Ivan Pavić, Josip Puškar, Ivan Soldo, Ivan Spasić

Repository contains source files for SPUS course project AY 2015/2016.

MotionDetection directory contains C++ project, and Makefile. Project can be
compiled on Linux based operating systems. Additional requirements are:

-> OpenCV 3.1 library
-> C++ 11

Tested on Debian based distributions (Debian 8.2 Jessie and Linux Mint)
Application is made for BeagleBone Black platform.


In MDInterface directory is Node.Js web application for monitoring results.
Requirements:

-> Node.Js v4.2.4 or better
-> mysql-server-5.5

Before running you should have prepared database and tables, from mysql
prompt run following commands:

-> create databse detection;
-> use detection;
-> CREATE TABLE motions (id INT NOT NULL AUTO_INCREMENT, 
                        image TEXT CHARACTER SET ascii,
                        date TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP, 
                        PRIMARY KEY(ID));
