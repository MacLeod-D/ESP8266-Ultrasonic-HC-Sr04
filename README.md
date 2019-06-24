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

Here I use my **CoopOS** to start different Tasks and the use of interrupts.  

The first level is an Interrupt routine:

    // Interrupt as response "Echo"of 10µs "Trigger" signal
    void IRQ_Echo() {
    IrqCount++;
        if (*GPIO_IN & 1) {                 // this is rising edge a pin 0 = D3
            mStart=micros();  
        }
        else {
            mEnd=micros();                 // falling edge
            IrqActive=false;               // IRQ measurement is reaDY
        }
    }

It is a "CHANGE" interrupt which is called by the Echo-evens mStart and mEnd of the HC-SR04.

To produce such interrupts the HC-SR04 must get a 10 µs pulse at "Trigger" line.

This is done by **CoopOS** Task_10us:

A measurement cycle is started from **Task1** :
- set IrqActive=true;
- sends 10µs Trigger signal
- waits for IrqActive=LOW  OR   Timeout
- filters the result:

<code>
    //-----------------------------
    // Send 10 µs High-Pulse and
    // wait for irq ready, delay, then next pulse
    
    uint32_t Task_10us() {
    static int counter;  
      taskBegin();
      // send one 10µs pulse, then stop
      while(1) {
        IrqActive=true;                   // reset by IRQ falling edge
        GPOS = (1<<D5);                   // send 10µs pulse
        taskDelay(10);
        GPOC = (1<<D5);
    
        // wait on answer of interrupt routine - timeout 100*1000 µs =100ms
        counter=0;
        while((IrqActive==true) && (counter<100)) {
          taskDelay(1000); counter++;
        }
    
        if (counter<100) {                // no timeout
          diff= (double)(mEnd-mStart);
          if (diff<20000) {
            value=value*0.9 + (0.1*diff); // filtering for noise reduction
          }
        }
        else value=0;                     // timeout: no answer pulse
            
        taskDelay(20000);
        if(diff<2000) {                   // small distances need extra delay !
          taskDelay(10000);
        }
    
        // Next pulse is initiated now
        
      }
      taskEnd();
    }



    if (counter<100) {                // no timeout
      diff= (double)(mEnd-mStart);
      if (diff<20000) {
        value=value*0.9 + (0.1*diff); // filtering for noise reduction
      }
    }
</code>

and starts again after a delay of 20 ms (30 ms if the distance to measure is small)

About 30 measurements per second are done.


The time mStart and mEnd are measured with an interrupt routine, which fires at both edges.<br>
It tests the edge(up,down) and remebers mStart, mEnd.<br>

<h3>Task2</h3>
Task2 is not involved in measurements! It even does not know, if there are measurements.

It just reads the values produces by Task1 and prints the results every 500 ms (twice a second) an shows some alarms like
- Timeout (val==0)
- distances lower than 300 mm (as an example)
- when something changes the measured distance suddenly (like a cat runnung through the echo path ;)


<h3>MySer and CoopOS (Task3></h3>
Serial output is a bottleneck.<br>
Here the output is done with a subclass of *Stream* and is written to OutBuf[] which is much faster.<br>

The Task **MySer** prints the contents of the buffer - every 50 µs one character.<br>
So the gaps in calling the *Scheduler* are reduced to about a maximum of 14µs.

*MyYser* stretches the  serial output to one character every 50 µs and write to the Uart-Fifo directly.<br>
The Scheduler here is called every 3 µs - when serial output is done it is prolonged to 40 µs.<br>
That is acceptable in most programs.<br>

** CoopOS cycle time during serial output:**<br>
![Image3](./image3.jpg?raw=true "Image3")

![Doxgen](./doc/doxy/index.html?raw=true "Doxygen Output")



 **CoopOS** does cooperative multitasking written in pure ANSI-C.   
 - It is *very* fast  (here up to 200000 task switcher per second).
 - has a small footprint (here 28 bytes per task ).  
 - reliable   
 - using microseconds, not milliseconds !
 - Less than 100 lines of code

**Warning**
CoopOS is NOT intended to do numbercrunching !<br>
A lot of time (more than 50%) is used to manage itself.<br>
That's not a failure but a feature;<br>

It is not possible to reach more than 160000 taskswitches per second using only a little amount of processor time.<br>

But  it is possible to test for instandce a whole port and start - if any pin has changed - a task blocked until then.<br>
You can do it *before* the scheduler is doing its normal work and that can be as fast as an interrupt - but you have all the possibilities>br>
of a task instead of an interrupt routine.<br>
That is possible because the Scheduler() is a small function in your source code - not a big library. So it is easy to customize it for your needs!<br>

The goal is to call as many tasks (or parts of it) as possible, run them very often and with reliable timings and low jitter.<br>
Useful examples:  Sample and filter values from an AD-Converter, using Rotary Dials, get serial input as commands,<br>
working with Shift Registers, running stepper motors ...etc.<br>
This is for all the embedded systems, which do NOTHING most of the time. And that is right for most of them.>br>


**Note**
There is an Esp8266 freeRTOS SDK version<br>
Test it - if it may fulfills your timing neeeds!


![CoopOS](./CooperativeMT.png?raw=true "Cooperative Multitasking")

