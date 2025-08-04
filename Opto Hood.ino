/*
 * Opto_Hood
 * 
 * This piece of code controls lights for optogenetic stimulation of flies in DAM monitors for simultaneous sleep/activity recording
 * Four DAM monitors(single beam) are kept in 3d printed "hoods" which have red leds. The arduino controls the power to the hoods through MOSFET/SSRs
 * 
 * General Scheme:                  
 *                           Arduino
 *                              :
 *                              :
 *                              v
 * LED Driver/SMPS ------>MOSFET/Solid State Relay ------> LEDs("Hood")
 * 
 * 
 * The code takes user inputs for time and frequency for each of the channels and then strobes the LEDs accordingly using a non-blocking while loop.
 * Frequencies at higher values might be erroneous as the time taken for each iteration of the loop(calling millis,comparisons,toggling pins) might be too long
 * Generally it is accurate till 50Hz
 * 
 * FOR THOSE WHO WANT MORE:
 * 
 * 
 * 
 * PWM (analogWrite()) won't work as it changes the duty cycle
 * 
 * micros() causes its own problems.First it is not as fast as millis() (reading a register vs. counting overflows). 
 * Also it overflows in little over an hour so extra code is needed to handle such situations.
 * 
 * If high freq is desired then one can use the tone() function (check documentation for reasonable limits) and control all channels at the same rate.
 * This does not allow control over individual channels
 * 
 * It might be possible to use the Timer1 overflow interrupts to control the channels at two different frequencies at fast enough rates.
 * 
 * 
 * Wriju
 * Chouhan Lab
 * TIFR-DBS
 * 25th July 2024
 * 
 * (working)
 * 
 */




const int Ch1 = 7;
const int Ch2 = 6;
const int Ch3 = 5;
const int Ch4 = 4;

String Command;

unsigned long start_time;

unsigned long Ch1_time;
unsigned long Ch2_time;
unsigned long Ch3_time;
unsigned long Ch4_time;
bool Strobe = true;
bool foreverOn = false;
unsigned long Stim_Dur = 0;
unsigned int Stim_Freq = 0;
//unsigned int Ch1_Freq;
//unsigned int Ch2_Freq;
//unsigned int Ch3_Freq;
//unsigned int Ch4_Freq;



void setup() 

{
  pinMode(Ch1, OUTPUT);
  pinMode(Ch2, OUTPUT);
  pinMode(Ch3, OUTPUT);
  pinMode(Ch4, OUTPUT);


  Serial.begin(9600);
  Serial.println("Starting......!!!");
  Serial.setTimeout(60000);
  delay(1000);


  if(Serial)                                                        //Serial.available gave problems
  {
    Serial.println("Enter Programming Mode? y/n");
    while(true)
       {        
        Command = Serial.readStringUntil('\n');
        if(Command.equals("y"))
          break;
        }   

    while(true)
        {
         Serial.println(" Enter Stimulation Duration(in minutes): Enter zero for continuous stim");
      
         Command = Serial.readStringUntil('\n');
         if(Command.toInt()> 0)
        {
          Stim_Dur = Command.toInt();   
          break;
        }
         
        else if(Command.toInt() == 0)
        {
          foreverOn = true;
          break;
        }
         
         else
         
          Serial.println(" Wrong Input! ");
          
        }    


    while(true)
        {
         Serial.println(" Choose Stimuation Type: ");
         Serial.println(" 1. Strobe ");
         Serial.println(" 2. Continuous Stimulation ");
      
         Command = Serial.readStringUntil('\n');
         if(Command.toInt() == 1)  
        {
          Strobe=true;
          while(true)
        {
         Serial.println(" Enter Stimulation Frequency (in hertz) for Channel 1 ");
      
         Command = Serial.readStringUntil('\n');
         if(Command.toInt()>0)  
       {
          //Stim_Freq = Command.toInt();

          Ch1_time = (1000/Command.toInt())/2;      //converting frequency in hertz to time period then deviding by 2 to get half time period
          break;
       }

         else
          Serial.println(" Wrong Input! ");

        }



         while(true)
        {
         Serial.println(" Enter Stimulation Frequency (in hertz) for Channel 2 ");
      
         Command = Serial.readStringUntil('\n');
         if(Command.toInt()>0)  
       {
         // Stim_Freq = Command.toInt();

          Ch2_time = (1000/Command.toInt())/2;  //converting frequency in hertz to time period then deviding by 2 to get half time period
          break;
       }

         else
          Serial.println(" Wrong Input! ");

        }

          while(true)
        {
         Serial.println(" Enter Stimulation Frequency (in hertz) for Channel 3 ");
      
         Command = Serial.readStringUntil('\n');
         if(Command.toInt()>0)  
       {
          //Stim_Freq = Command.toInt();

          Ch3_time = (1000/Command.toInt())/2;      //converting frequency in hertz to time period then deviding by 2 to get half time period
          break;
       }

         else
          Serial.println(" Wrong Input! ");

        }

          while(true)
        {
         Serial.println(" Enter Stimulation Frequency (in hertz) for Channel 4 ");
      
         Command = Serial.readStringUntil('\n');
         if(Command.toInt()>0)  
       {
          //Stim_Freq = Command.toInt();

          Ch4_time = (1000/Command.toInt())/2;     //converting frequency in hertz to time period then deviding by 2 to get half time period
          break;
       }

         else
          Serial.println(" Wrong Input! ");

        }


        break;
        } // end of acquiring channel frequencies

          else if(Command.toInt()== 2)  
       {
          Strobe = false;
          break;
        }

        }
      
         while(true)
        {
         Serial.println("Start? y/n");
      
         Command = Serial.readStringUntil('\n');
        if(Command.equals("y"))
          break;
         
        }
        


     

    
  }// end of acquiring user inputs


  Stimulate();

  
}



void loop() 
{
  // put your main code here, to run repeatedly:

}

void Stimulate()
{
   //Serial.println(Ch1_time);
  // Serial.println(Ch2_time);
   Serial.println("Starting with Stimulation!!!");
   start_time = millis();
   
   unsigned long Ch1_start_time = start_time;                     // start_time for each channel is the half time period
   unsigned long Ch2_start_time = start_time;                     
   unsigned long Ch3_start_time = start_time;
   unsigned long Ch4_start_time = start_time;
  

  
   bool Ch1_State = 0;
   bool Ch2_State = 0;
   bool Ch3_State = 0;
   bool Ch4_State = 0;



   

  

Stim_Dur = Stim_Dur*1000*60;           // convert mins to millisecs
             
if(Strobe)
{
 //Serial.println("Inside Strobe");
 
while((millis() - start_time) < Stim_Dur || foreverOn)          // this loop controls the total duration of Stimulation
  {                                                             //Time_Now is refreshed at each iteration
   unsigned long Time_Now = millis();                           //calling millis is time consuming so the value is stored in one variable.
  
   /*Serial.print(Time_Now);
    Serial.print("          ");
   Serial.print(Ch1_start_time);
    Serial.print("          ");                   //Printing to serial takes too much time so the frequencies would be off. Try not to print in the loop
   Serial.print(Ch1_time);
    Serial.print("          ");
    Serial.print(Ch4_start_time);
    Serial.print("          ");
   Serial.print(Ch4_time);
   Serial.println();
*/
 

      if(Time_Now - Ch1_start_time > Ch1_time)            // if this difference is greater then the state of the switch is toggled
      {
              
       Ch1_State = !Ch1_State;
       digitalWrite(Ch1, Ch1_State);
       digitalWrite(13, Ch1_State);
      
       
       Ch1_start_time = Ch1_start_time + Ch1_time;                         // start_time is refreshed at every state change
      }
    

    
   
      if(Time_Now - Ch2_start_time > Ch2_time)            
      {
              
       Ch2_State = !Ch2_State;
       digitalWrite(Ch2, Ch2_State);
       Ch2_start_time =Ch2_start_time+Ch2_time;                         
      }
  
   
      if(Time_Now - Ch3_start_time > Ch3_time)            
      {
              
       Ch3_State = !Ch3_State;
       digitalWrite(Ch3, Ch3_State);
       Ch3_start_time =Ch3_start_time+Ch3_time;                         

      }



      if(Time_Now - Ch4_start_time > Ch4_time)           
      {
              
       Ch4_State = !Ch4_State;
       digitalWrite(Ch4, Ch4_State);
       Ch4_start_time =Ch4_start_time+Ch4_time;                        
      }


    
  }//end of while

}// end of strobe

else
   while((millis() - start_time) < Stim_Dur || foreverOn)  
   {
    digitalWrite(Ch1, HIGH);
    digitalWrite(Ch2, HIGH);
    digitalWrite(Ch3, HIGH);
    digitalWrite(Ch4, HIGH);
   }


    digitalWrite(Ch1, LOW);
    digitalWrite(Ch2, LOW);
    digitalWrite(Ch3, LOW);
    digitalWrite(Ch4, LOW);


   Serial.println("End of Stimulation!!!");
  
}// end of Stimulate
