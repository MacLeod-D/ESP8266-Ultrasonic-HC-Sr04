/**@file*/

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
It tests the edge(up,down) and remebers mStart, mEnd.<br>

<h3>Task2</h3>
Prints the results every 500 ms (twice a second) an shows some alarms like
- Timeout (val==0)
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

 //#include "doxy.h"
 



#include <stdlib.h>
#include <stdio.h>
//#define LINUX
//#define ARDUINO
//#define ESP32


#define MAXTASKS 10

#include "MySerial.h"


//static const uint8_t D0   = 16;
//static const uint8_t D1   = 5;
//static const uint8_t D2   = 4;
//static const uint8_t D3   = 0;
//static const uint8_t D4   = 2;
//static const uint8_t D5   = 14;
//static const uint8_t D6   = 12;
//static const uint8_t D7   = 13;
//static const uint8_t D8   = 15;
//static const uint8_t D9   = 3;
//static const uint8_t D10  = 1;

// for direct Pin manupulation
#define GPIO_IN ((volatile uint32_t*) 0x60000318)


//-----------------------------
//          GLOBALS
//-----------------------------
typedef unsigned long long uint64_t;
int               nTasks;                         // number of tasks
int               thisID;                         // number of running task
unsigned long    SchedulerCalls; 
unsigned long    T1Counter, T2Counter;

// possible states of a task:
enum TState { READY, DELAYED, BLOCKED };
uint32_t Count6;

int             interruptPin=0;
int             T_10us;
volatile uint32_t mStart,mEnd;
volatile bool   IrqActive=false;
uint64_t        IrqCount;                         // show interrupt #
double          value;                            // last distance in mm (average)
double          diff;                             // last distance in mm

/** \page ts TaskSwitch -Macros
 - taskBegin()<br>
 All tasks maust have this before while(1)-loop !!!

 - taskEnd()<br>
 All tasks maust have this after while(1)-loop !!!

 - taskSwitch()<br>
 Back to Scheduler

 - taskDelay(n)<br>
 Back to Scheduler and stop this tas for n microseconds

 - taskBlock(n)<br>
 Back to Scheduler and block the task number ID=n until resumed
 
 - taskResume(n)<br>
 Back to Scheduler and set the task number ID=n to READY
 
*/


#define taskBegin()                                                            \
static int _mark = 0;                                                          \
  switch (_mark) {                                                             \
  case 0:
  
  
#define taskEnd()                                                              \
  _mark = 0;   return -1;                                                      \
}
  
#define taskSwitch()  _mark = __LINE__;     return(0) ;  case __LINE__:;         
#define taskDelay(VAL)  _mark = __LINE__;   return(VAL) ;  case __LINE__:;         
                                                   
#define taskBlock(ID)   _mark = __LINE__;   Tasks[ID].State = BLOCKED;  
#define taskResume(ID)  Tasks[ID].State = READY; Tasks[ID].Delay = 0;
//-----------------------------
//    Structure of a Task
//-----------------------------
    

  
struct task{
  unsigned int ID;
  const char*  Name;
  volatile TState       State;
  uint32_t     LastCalled;   // µs
  uint32_t     Delay;        // µs
  uint32_t     (*Func)();
  int          Priority;
};
struct task Tasks[MAXTASKS];
  
//-----------------------------
//   Init of Tasks
//-----------------------------
  
int InitTask( const char *name, uint32_t (*func)(void), int Priority  ) {
struct task *thisTask;
  if (nTasks== MAXTASKS) {
    printf("Only %d Tasks allowed! ", MAXTASKS);
    return -1;
  }
  printf("Init: %s function pointer: 0X%lX\n", name, (uint32_t)func);
  
  thisTask=                 &Tasks[nTasks];
  thisTask->Name =          name;
  thisTask->ID =            nTasks;
  thisTask->State =         READY;
  thisTask->LastCalled =    micros();
  thisTask->Delay =         0;
  thisTask->Func =          func;
  thisTask->Priority =      Priority;
  nTasks++;
  return nTasks-1;
}    
//-----------------------------
//    Scheduler
// call tasks an saves state
//-----------------------------
void Scheduler() {
uint32_t result;
struct task *thisTask;
int Priority;
int MaxPriority, MaxPTask;

//  PORTB |= (1<<5);
  //digitalWrite(D6,HIGH);
  GPOS = (1<<D6);
  SchedulerCalls++;
  
  
//  PORTB &= ~(1<<5);
  
  Priority=0;
  MaxPriority=-1;
  MaxPTask=-1;

  
  
  for (int i=0; i<nTasks;i++) {
    thisTask=&Tasks[i];
    
    // 1. look for tasks which are READY now
    if (thisTask->State == DELAYED) {    // State: DELAYED -> READY
      if ((micros() - thisTask->LastCalled) > thisTask->Delay) {
        thisTask->Delay=0;
        thisTask->State=READY;
      }
    }
    
    // 2. look for the READY task with the highest priority
    if (thisTask->State == READY) {
      if (thisTask->Priority > MaxPriority) {
        MaxPriority = thisTask->Priority;
        MaxPTask=i;
      }
    }
  } // for

  //digitalWrite(D6,LOW);
  GPOC = (1<<D6);
  
  if (MaxPTask>=0) {
    thisTask=&Tasks[MaxPTask];
    thisID=MaxPTask;
    
//    if you want ot know when wich task is called:
//    uncomment these 2 lines and count the pulses with a LA:
//    for (int i= 0; i< MaxPTask+1; i++) { GPOS = (1<<D6); GPOC = (1<<D6);}
//    GPOS = (1<<D6);

    result=thisTask->Func();          // call the READY Task <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    
    thisTask->LastCalled = micros();
    GPOC = (1<<D6);
    MaxPriority=-1;                   
    MaxPTask=-1;
    
    
    // save result after task yields:
    // TaskSwitch: set READY if not BLOCKED
    if ((result == 0) && (thisTask->State!=BLOCKED)) {
      thisTask->State = READY;
    }
    else {
      // TaskDelay: set Delay-time and DELAYED
      if (result>0) {
        thisTask->State = DELAYED;      // State: READY -> DELAYED
        thisTask->Delay = result;
      }
     
    }
  }
  
}



      
//-----------------------------
//    Here are the tasks
//-----------------------------

/*


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


*/




    
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


// Print results and test Alarm
//-----------------------------
uint32_t Task2() {
static double val;
static double oldVal;
static double alarm;
  taskBegin();
 
  while(1) {
    cli();
    val=value*0.165;  //µs => mm
    sei();    
    
    MySerial.print("Distance:  "); 
    taskSwitch();
    MySerial.print(val);  
    taskSwitch();
    MySerial.print(" IrqCount: "); 
    taskSwitch();
    MySerial.print(IrqCount); 
    taskSwitch();
    MySerial.print(" TaskSwitches/s: "); 
    taskSwitch();
    MySerial.println(SchedulerCalls);
    SchedulerCalls=0;

    
    // Sudden difference:
    if (alarm>0.1*val) {
      MySerial.print("Moving Alarm:    "); 
      taskSwitch();
      MySerial.print(val); 
      taskSwitch();
      MySerial.println(" mm\n"); 
    }

    // lower limit: 30 cm
    if (val<300) {
      MySerial.print("Distance Alarm:  "); 
      taskSwitch();
      MySerial.print(val); 
      taskSwitch();
      MySerial.println(" mm\n"); 
    }
    taskDelay(500000);
  }
  taskEnd();
}




uint32_t MySer_Task() {
  taskBegin();
  while(1) {
    if (SerHead!=SerTail) {
      MySerial.toSer(OutBuf[SerTail++]);
      if (SerTail==SER_BUF_MAX) SerTail=0;
    }    
    taskDelay(50);
  }
  taskEnd();
}



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




void setup() {
  //pinMode(13,OUTPUT);
  pinMode(D6,OUTPUT);
  pinMode(D5,OUTPUT);
  pinMode(interruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(interruptPin), IRQ_Echo, CHANGE);

  Serial.begin(500000);
  MySerial.setSerial(&Serial); // Redirect Serial 
  Serial.printf(" *** (C) 2013-2019 Dipl. Phys. H. Weber\n");
  Serial.printf(" *** Principles of CoopOS\n");
  Serial.printf(" *** 3 Tasks running ...\n");
  Serial.printf(" *** One Task needs space of %d Bytes\n\n", sizeof (struct task) );
 
  Serial.println("Init Tasks ...");  
  T_10us=
  InitTask("T_10us",Task_10us,5);        // prepare the tasks
  InitTask("Task2",Task2,4);
  InitTask("MySer",MySer_Task,5);
  
  Serial.printf("Init Tasks ready!");
  Serial.flush();
 
}




void loop() {
  
// replace this to:
//   Scheduler();
// if you want to do some WLAN !
  
//------------
  while(1) {
    Scheduler();              // 3µs cycle time - 24µs when serial output
    ESP.wdtDisable();
  }
//-----------
}



