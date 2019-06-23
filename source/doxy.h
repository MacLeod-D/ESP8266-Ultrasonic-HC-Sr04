


/**
\mainpage  CoopOS_Simplest-Demo1 / ESP8266-Version / Arduino-IDE

Cooperative multitasking with Esp8266 (Arduino IDE)<br>
(C) 2013-2019 Helmut Weber<br>
<br>
\subpage purpose Purpose   

\subpage license License  

\subpage output Output

\subpage la Logic Analyzer

\subpage ts TaskSwitch - Macros   


\page license  License:


\code 
    Copyright (C) 2013-2019  H. Weber <Dph.HelmutWeber@Web.de>
    This program is free software: you can redistribute it and/or modify   
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    
     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.
    
     You should have received a copy of the GNU General Public License
     along with this program.  If not, see <http://www.gnu.org/licenses/>.
\endcode


\page purpose Purpose

<h3>Distance Measuring with Ulttrasonic HC-SR04 with ESP8266 using Arduino IDE</h3>

HC-SR04 gets a 10µs Trigger pulse, sends 8 cycles of 40 Khz tone and:
- sets Echo signal HIGH
- sets Echo signal LOW, when Echo arrived:



    IrqActive:
       true                                 false
        ______________________________________
        |                                     |
    ____|                                     |_______
    
    Task1_10us:     
         ____
         |  |
    _____|  |_____
    
       
    Interrupt:
                _____________________________
                |                            |
    ____________|                            |_______
             mStart                        mEnd
    measured
    time:       <--------------------------->


<h3>Concept</h3>

A measurement cycle is started from **Task1** :
- set IrqActive=true;
- sends 10µs Trigger signal
- waits for IrqActive=LOW  OR   Timeout
- filters the result:


    if (counter<100) {                // no timeout
      diff= (double)(mEnd-mStart);
      if (diff<20000) {
        value=value*0.9 + (0.1*diff); // filtering for noise reduction
      }
    }

and starts again after a delay.   

About 30 measurements per second are done.


The time mStart and mEnd are measured with an interrupt routine, which fires at both edges.<br>
I tests the edge(up,down) and remebers mStart, mEnd.<br>

<h3>Task2</h3>
Prints the results every 500 ms (twice a second) an shows some alarms like
- Timeout (vla==0)
- distances lower than 300 mm (as an example)
- when something changes the measured distance suddenly (like a cat runnung through the echo path ;)


<h3>MySer</h3>
Spreads serial output to one character every 50 µs and write to the Uart-Fifo directly.<br>
Serial output is a bottleneck of multitasking.<br>
The Scheduler here is called every 3 µs - when serial output is done it is prolonged to 24 µs.<br>
That is acceptable in most programs.<br>



 CoopOS does cooperative multitasking written in pure ANSI-C.   
 - It is *very* fast  (here up to 200000 task switcher per second).
 - has a small footprint (here 28 bytes per task ).  
 - reliable   
 - using microseconds, not milliseconds !

\warning 
CoopOS is NOT intended to do numbercrunching !<br>
A lot of time (more than 50%) is used to manage itself.<br>
That's not a failure but a feature;<br>

It is not possible to reach more than 200000 taskswitches per second using only a little amount of processor time.<br>

But  it is possible to test for instandce a whole port and start - if any pin has changed - a task blocked until then.<br>
You can do it *before* the scheduler is doing its normal work and that can be as fast as an interrupt - but you have all the possibilities>br>
of a task instead of an interrupt routine.<br>
That is possible because the Scheduler() is a small function in your source code - not a big library. So it is easy to customize it for your needs!<br>

The goal is to call as many tasks (or parts of it) as possible, run them very often and with reliable timings and low jitter.<br>
Useful examples:  Sample and filter values from an AD-Converter, using Rotary Dials, get serial input as commands,<br>
working with Shift Registers, running stepper motors ...etc.<br>
This is for all the embedded systems, which do NOTHING most of the time. And that is right for most of them.>br>


\note
There is an Esp8266 freeRTOS SDK version<br>
Test it - if it may fulfills your timing neeeds!


*/


/**

\page la Logic Ananlyzer

It is highly recommended to use a logic analyzer to test yout CoopOS programs.<br>
Those devices ar not so exdpensiv - really cheap, if you dare to buy chinese rebuilds.<br>


<h3> The measurement:</h3>
<br>
\image html image1.jpg
<br>
<h3>Task switching and serial output</h3>
\image latex image1.jpg



Here we can see how the output of a serial string adds 24 µs.<br>

\image html image2.jpg
\image latex image2.jpg




\page output  Output

Serial output is time consuming. MySerial reduces the time from ms to about 40 µs.<br>
But that means that tasks like Task6 have to take into account that instead of<br>
100 µs cycle time you may have (in rare circumstances) 150 µs.<br>
Do not use serial output or take it.


This is a sample output of the program

>

            OUTPUT
        
     *** (C) 2013-2019 Dipl. Phys. H. Weber
     *** Principles of CoopOS
     *** 3 Tasks running ...
     *** One Task needs space of 28 Bytes
    
    Init Tasks ...
    Init: T_10us function pointer: 0X40202084
    Init: Task2 function pointer: 0X402024A4
    Init: MySer function pointer: 0X40202280
    Init Tasks ready!Distance:  0 1
    Distance Alarm:  0 mm
    
    Distance:  1133,3 36
    Moving Alarm:    1133,3 mm
    
    Distance:  1299,4 70
    Moving Alarm:    1299,4 mm
    
    Distance:  1327,0 104
    Distance:  1331,2 139
    Distance:  1331,6 169
    Distance:  869,1 196
    Moving Alarm:    869,1 mm
    
    Distance:  1255,3 230
    Moving Alarm:    1255,3 mm
    
    
    Distance:  1333,5 678
    Distance:  1333,9 712
    Distance:  1333,9 747
    Distance:  1334,4 782
    Distance:  1335,1 816
    Distance:  1334,6 850
    Distance:  1334,7 884
    Distance:  1098,3 918
    Moving Alarm:    1098,3 mm
    
    Distance:  930,9 951
    Moving Alarm:    930,9 mm
    
    Distance:  1272,3 986
    Moving Alarm:    1272,3 mm
    
    Distance:  1324,0 1.020
    Distance:  1331,2 1.054
    Distance:  1333,2 1.089
    Distance:  1333,9 1.124
    Distance:  1334,4 1.158
    Distance:  1334,6 1.192


\page la Logic Analyzer

For analysing a multitasking system you should use a logic analyzer or at leat a scope.<br>
<br>


*/
