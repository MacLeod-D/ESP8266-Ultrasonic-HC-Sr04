// Redirect   Serial.print    to a buffer, which is red by tasK
// (C) 2019 Helmut Weber 

// MySerial:
// Redirect Serial to any destination
#define SER_BUF_MAX 300

volatile int SerHead, SerTail;
char OutBuf[SER_BUF_MAX];


/**
 * \brief
 * "_itoa"<br> 
 * This is the selfmade conversion from unsigned int to ascii-string<br>
 * digits are the number of digits behind the"."
 */
char *_itoa(unsigned int l) {
static char b[31];
static char const digit[] = "0123456789";
char* p = b;
uint32_t i;


  //i=(uint32_t)f;    
  
  p=b+28;
  *p = 0;
  *(p+1)=0;
  int cnt=0;
  do { //Move back, inserting digits as u go
      cnt++;
      // for smarter printout of long:
      if (cnt == 4) { p--; *p='.'; cnt=1; }
      p--;
      *p = digit[l % 10];
      l = l/10;
  } while(l);

  return p; // return result as a pointer to string
}


/**
 * \brief
 * "ltoa"<br> 
 * This is the selfmade conversion from unsigned long to ascii-string<br>
 * digits are the number of digits behind the"."
 */
char *ltoa(unsigned long l) {
static char b[31];
static char const digit[] = "0123456789";
char* p = b;
uint32_t i;


  //i=(uint32_t)f;    
  
  p=b+28;
  *p = 0;
  *(p+1)=0;

  uint8_t cnt=0;
  do { //Move back, inserting digits as u go
      cnt++;
      // for smarter printout of long:
      if (cnt == 4) { p--; *p='.'; cnt=1; }
      
      p--;
      *p = digit[l % 10l];
      l = l/10l;
  } while(l);

  return p; // return result as a pointer to string
}



/**
 * \brief
 * "ftoa"<br> 
 * This is the selfmade conversion from float to ascii-string<br>
 * digits are the number of digits behind the"."
 */
char *ftoa(double f, int digits) {
static char b[31];
static char const digit[] = "0123456789";
char* p = b;
uint32_t i;

int d,j;
  d=digits;  
  while (d) {
    f*=10.0;
    d--;
  }
  
  i=(uint32_t)f;    
  
  p=b+28;
  j=0;
  *p = 0;
  *(p+1)=0;
  
  do { //Move back, inserting digits as u go
      if (j == digits) { p--; *p=','; }
      p--;
      *p = digit[i % 10ll];
      i = i/10ll;
      j++;
  } while(i);

  return p; // return result as a pointer to string
}





class mySerial
{
  private:
    Stream *mystream;
  
  public:
    void setSerial(Stream *streamObject)
    {
      mystream=streamObject;
    }

// ------ Only these 2 must be redirected !!! ----
    void inline write( byte b) {
       //mystream->write(b);
    }

    void inline toSer( char c) {
      //Serial.write((char)c);
// Arduinio
//      // wait until Transmitter empty
//      while ( !( UCSR0A & (1<<UDRE0)) );
//      // send one byte
//      UDR0 = (uint8_t)c;


 // Esp8266: wtite direct to FiFo:     
      U0F=c;
      
      
     
    }

    void inline write( char c) {
       //mystream->write(c);
      OutBuf[SerHead++]=c;
      if (SerHead==SER_BUF_MAX) SerHead=0;
    } 
//------------------------------------------------
   
    void println() {
      write('\n');
    }

    void inline print(char *str) {
    register char *pt=str;
     while(*pt) { write(*pt++);  }
    }

    void inline println(char *str) {
      print(str);
      println();
    }

    
 
    void print(unsigned int i) {
    char buf[20];  
      itoa(i, buf, 10);
      print(buf);
    }

    void println(unsigned int i) {
      print(i);
      println();
    }

    void print(uint8_t i) {
    char buf[20];  
      itoa(i, buf, 10);
      print(buf);
    }

    void print(uint8_t i, uint8_t n) {
    char buf[20];  
      itoa(i, buf, n);
      print(buf);
    }

    void print(unsigned int i, uint8_t n) {
    char buf[20];  
      itoa(i, buf, n);
      print(buf);
    }

    void println(unsigned int i, int n) {
      print(i,n);
      println();
    }

    void println(uint8_t i) {
      print(i);
      println();
    }

    void print(int i) {
    char buf[20];  
      itoa(i, buf, 10);
       print(buf);
    }

    void println(int i) {
      print(i);
      println();
    }


    void print(unsigned long i) {
//    char buf[20];  
      //ltoa(i, buf, 10);
       print(ltoa(i));
    }

    void println(unsigned long i) {
      print(i);
      println();
    }


    void print(long i) {
    char buf[20]; 
      ltoa(i, buf, 10);
      print(buf);
    }

    void println(long i) {
      print(i);
      println();
    }

    void print(uint64_t  i) {
    char buf[20]; 
      //ltoa(i, buf, 10);
      //print(buf);
      print(ltoa(i));
    }

    void println(uint64_t i) {
      print(i);
      println();
    }

    
    void print(float i) {
      print(ftoa(i,1));
    }

    void println(float i) {
      print(i);
      println();
    }

    void print(double i) {
      print(ftoa(i,1));
    }

    void println(double i) {
      print(i);
      println();
    }


    char read() {
      return mystream->read();
    }

    bool available() {
      return mystream->available();
    }

    void flush() {
      mystream->flush();
    }


// to do:        more types: float, double
//               even custom types are possible !


    
};




mySerial MySerial;



