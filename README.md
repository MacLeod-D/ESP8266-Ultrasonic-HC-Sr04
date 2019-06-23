# ESP8266-Ultrasonic-HC-Sr04
#Esp8266 #Interrupts #CoopOS #NoiseReduction

<h3>Distance Measuring with Ulttrasonic HC-SR04 with ESP8266 using Arduino IDE</h3>

HC-SR04 gets a 10µs Trigger pulse, sends 8 cycles of 40 Khz tone and:
- sets Echo signal HIGH
- sets Echo signal LOW, when Echo arrived:

<code>

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
</(code>

<h3>Concept</h3>

Here I use my **CoopOS** to start different Taks:  


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
It tests the edge(up,down) and remebers mStart, mEnd.<br>

<h3>Task2</h3>
Prints the results every 500 ms (twice a second) an shows some alarms like
- Timeout (val==0)
- distances lower than 300 mm (as an example)
- when something changes the measured distance suddenly (like a cat runnung through the echo path ;)


<h3>MySer and CoopOS</h3>
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

