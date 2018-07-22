Author: Brian Leeson

**Description**  
This was a class project for my CIS 415, Operating Systems course. We were meant to
write a network driver that took information off a network and gave it to waiting 
applications. The applications were able to put information on to the network by going
through our driver. This passing of data was accomplished using pthreads to manage 
independent tasks and a bounded queuing systems to store packets of information as they
traverse the driver. 

I wrote networkdriver.c as defined by PROJ2SPEC.pdf. This required an understanding of the
other files given to us for this assignment.

**Dependencies**  
* This program was developed on Arch Linux, and has been run on Ubuntu 14/16/18. 
It should work on most Linux systems.
* make
* gcc

**Use**  
When running this program will output a log of where packets are heading, along with any errors
that may occur. Some of the log statements were created by the instructor of this course and
some by myself

$ make mydemo  
$ ./mydemo

