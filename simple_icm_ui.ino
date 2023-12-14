#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <AccelStepper.h>
#include <MultiStepper.h>
#include <math.h>
#include <EEPROM.h>
#include "SoftwareSerial.h"

//simple icm, only 1 motor

// Motor Parameters
#define MOTOR_STEPS 200 //
#define RPM 1000
#define MS_STEP 8 // default 1/8 microstep, 200*8 = 1600 microstepping
#define CALI_ACCEL 400


//Display Pins
#define TFT_CS   9
#define TFT_RST  12  // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC   8
#define TFT_MOSI 11  // Data out
#define TFT_SCLK 10 // Clock out

/* Motor Pins */
#define rear_DIR    6
#define rear_STEP   7
#define front_DIR   0
#define front_STEP  1

//Display Joystick 
#define UP_BUTTON 2
#define DOWN_BUTTON 18
#define LEFT_BUTTON 16
#define RIGHT_BUTTON 20
#define CENTER_BUTTON 3
#define A_BUTTON 15
#define B_BUTTON 17
#define X_BUTTON 19
#define Y_BUTTON 21

/*RX and TX for JQ8900 communication*/
//It's important to note that those RX and TX labels are with respect to the device itself. 
//So the RX from one device should go to the TX of the other, and vice-versa
#define HBYTE(X) ((X & 0xFF00) >> 8) 
#define LBYTE(X) (X & 0xFF) 
#define RX 5
#define TX 4

//control camera shutter  - may need change FOCUS_CAMERA GPIO PIN
#define FOCUS_CAMERA 21 //26
#define SHUTTER_CAMERA 22 //27

/* Colour Strings */
#define WHITE     0xFFFF
#define BLACK     0x0000
#define AQUA      0xFEC0
#define DEEPPINK  0xF8B2  
#define DARKGREEN 0x0320
#define RED       0xF800 
#define LIGHTSKYBLUE 0x867F 
#define SNOW 0xFFDF
#define VIOLET 0xEC1D 
#define YELLOWGREEN 0x9E66
#define GOLDENROD 0xF81F
#define AZURE 0xF7FF
#define CORAL 0xFBEA 
#define CADETBLUE 0x5CF4
#define LIME 0x07E0

/* Symbols */
#define UP_ARROW    0x18
#define DOWN_ARROW  0x19
#define RIGHT_ARROW 0x1A
#define LEFT_ARROW  0x1B
#define SELECT      0x2A


/* Things that are saved
 *  - lens_range     (Max range of lens)
 *  - lens_current    (Current main value)
 *  - orientation     (Orientation of motors)
 *  - shutter_speed   (Shutter speed)
 *  - motor_time
 */
int lens_range = 0; 
int lens_current = 0;  
int orientation = 0;    
int shutter_time = 0;
int motor_time = 0;   
int excess_option_set = 0; //default
int camera_shutter_open = 0; //default 0 is not open


// Global Variables
int updateMenu = true;
int max_option = 0;
int home_screen = -1;
int configuration_screen = -1;
int camera_setting_screen = -1;
int camera_positioning_screen = -1;
int motor_calibration_screen1 = -1;
int options_menu1 = -1;
//int motor_calibration_screen2 = -1;
int excess_option_screen = -1;
int action_screen_1 = -1;
int lens_movements_menu1 = -1;
int lens_movements_menu2 = -1;
int fixed_paterns_menu1 = -1;
int fixed_paterns_menu2 = -1;


//shutter_time
int max_shutter_time = 60;

//symbol setting for display,calibration,interface ui etc,...
int symbol_padding = 40;
int symbol_size = 20;


/* Motor Objects*/
AccelStepper main_motor(AccelStepper::DRIVER, rear_STEP, rear_DIR);


/* Display Object, set up the displat init */
Adafruit_ST7789 tft(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

//Comminication to JQ8900
SoftwareSerial SoftwareSerial(RX,TX); // RX, TX

/***** Defining Strings on Display *****/

//home_screen options focus on configuration,
const char home_header[] PROGMEM = "|- Home Menu -|";
const char home_0[] PROGMEM = "Camera Config";
const char home_1[] PROGMEM = "Actions-Simple";
const char home_2[] PROGMEM = "Actions-Pattern";

//main_menu1 options focus on configuration,
const char mm_configuration_header[] PROGMEM = "|- Configuration Menu -|";
const char mm_configuration_0[] PROGMEM = "Camera Settings";
const char mm_configuration_1[] PROGMEM = "Motor Calibration";
const char mm_configuration_2[] PROGMEM = "Options";


////main_menu2 options focus on actions,
const char mm_action1_header[] PROGMEM = "|- Action Menu-1 -|";
const char mm_action1_0[] PROGMEM = "Movements";


/////Calibration/////
const char cs_name[] PROGMEM = "|- Camera Settings -|";
const char cs_0[] PROGMEM = "1-Shutter Time"; //Camera Shutter Time, Shutter Speed
const char cs_1[] PROGMEM = "2-Motor Time";
const char cs_2[] PROGMEM = "3-Excess";

//position orientation 
const char pm_name[] PROGMEM = "|- Positioning Setting -|";
const char pm_0[] PROGMEM = "orientation-0";
const char pm_1[] PROGMEM = "orientation-1";

const char shutter_menu[] PROGMEM = "|- Shutter Time(s) -|";
const char motor_time_menu[] PROGMEM = "Motor Move Time(s)";

//motor callibration
const char mc1_name[] PROGMEM = "|- Calibration -|";
const char mc1_0[] PROGMEM = "Lens_Cali";
const char mc1_1[] PROGMEM = "POV_Cali";


//options page
const char option_name[] PROGMEM = "|- Options -|";
const char option_0[] PROGMEM = "Rotation";
const char option_1[] PROGMEM = "RESET Cam";
const char option_2[] PROGMEM = "RESET Cali";

//clibrating ZF len(s)
const char cali_lens[] PROGMEM = "|--Calibrate Lens --|";
const char string_cali_initial_position[] PROGMEM = "Initial Position (zero)";
const char string_cali_final_position[] PROGMEM = "Final Position (max)";
const char string_cali[] PROGMEM = "Move joystick to set";
const char string_left[] PROGMEM = "  Min";
const char string_right[] PROGMEM = "  Max";


const char adjust_lens[] PROGMEM = "|--- Adjust Lens ---|";
const char string_36[] PROGMEM = "Adjust lens";
const char string_37[] PROGMEM = "to the desired Image";
const char string_38[] PROGMEM = "to desired Outcome";

//excess option
const char excess_option_name[] PROGMEM = "|- Excess -|";
const char excess_option_0[] PROGMEM = "Pre";
const char excess_option_1[] PROGMEM = "Split";
const char excess_option_2[] PROGMEM = "After";

//lens page1
const char lens_m1_name[] PROGMEM = "|- Lens Movements -|";
const char lens_m1_0[] PROGMEM = "Max";
const char lens_m1_1[] PROGMEM = "Min";
const char lens_m1_2[] PROGMEM = "Max&Back";
const char lens_m1_3[] PROGMEM = "Min&Back";
//lens page2
const char lens_m2_name[] PROGMEM = "|- Lens Movements -|";
const char lens_m2_0[] PROGMEM = "Value";
const char lens_m2_1[] PROGMEM = "Value&Back";

//presets page1
const char preset1_name[] PROGMEM = "|----- Presets -----|";
const char preset1_0[] PROGMEM = "Bokeh";
const char preset1_1[] PROGMEM = "Firework";
const char preset1_2[] PROGMEM = "ZoomBlur";
const char preset1_3[] PROGMEM = "SineWave ";

//presets page2
const char preset2_name[] PROGMEM = "|----- Presets -----|";
const char preset2_0[] PROGMEM = ".....";

//countdown
const char counttext_1[] PROGMEM = "Get Ready!";
const char counttext_2[] PROGMEM = "3";
const char counttext_3[] PROGMEM = "2";
const char counttext_4[] PROGMEM = "1";
const char counttext_5[] PROGMEM = "SNAP!";

/* String Table */
const char *const home_menu[] PROGMEM = {home_0, home_1,home_2}; //Home_menu table
const char *const main_menu_1[] PROGMEM = {mm_configuration_0, mm_configuration_1, mm_configuration_2}; //main_menu1 table
const char *const main_menu_2[] PROGMEM = {mm_action1_0}; //main_menu2 table

const char *const camera_settings_menu[] PROGMEM = {cs_0, cs_1, cs_2};
const char *const positioning_menu[] PROGMEM = {pm_0,pm_1};
const char *const motor_calibration_menu1[] PROGMEM {mc1_0, mc1_1};
const char *const options_menu[] PROGMEM {option_0, option_1,option_2};
const char *const excess_option_menu[] PROGMEM {excess_option_0, excess_option_1,excess_option_2};

const char *const lens_menu1[] PROGMEM = {lens_m1_0,lens_m1_1,lens_m1_2,lens_m1_3};
const char *const lens_menu2[] PROGMEM = {lens_m2_0,lens_m2_1};


const char *const calilens_left[] PROGMEM = {cali_lens, string_cali_initial_position, string_cali, string_left};
const char *const calilens_right[] PROGMEM = {cali_lens, string_cali_final_position, string_cali, string_right};

const char *const lens_adjust[] PROGMEM = {adjust_lens, string_36, string_37};
const char *const lens_dist[] PROGMEM = {lens_m1_2, string_36, string_38};

const char *const preset1_menu[] PROGMEM = {preset1_0, preset1_1, preset1_2, preset1_3};
const char *const preset2_menu[] PROGMEM = {preset2_0};

const char *const countdown[] PROGMEM = {counttext_1, counttext_2, counttext_3, counttext_4, counttext_5};


// Function Declaration
void initializing_Page();
void updateScreen(float delay_ms=0);
int getUpDown(int max_option, int current_option, int delay_ms);
void resetToHomeScreen();
void countdownMenu();

void hotbar(const char title[], int current, int max_range, bool haveBack=false, int header=-1, int footer=-1, uint16_t color=WHITE, bool updateBar=false);
int getLeftRight_value(int range, int current, int low_limit, int delay_ms);
int get_camera_calibration_update();
int get_motor_calibration_update();

//void caliMenu(const char *const string_table[], int current_step, int max_steps, uint16_t color, bool updateBar);
void moveMotorMenu(int count, const char *const string_table[], int current_step, int max_steps, uint16_t color=WHITE, bool updateBar=false);

void printMoveSteps(const char title[], uint16_t color, int movement_display_option);
void setAccel(float accel);
void setCurrentPos(float value);

void moveMotor(int pos_desired, float motor_time = motor_time);
int chooseDist(int count, const char *const string_table[], bool goBack=false, uint16_t color=WHITE);
void goDist(const char title[], int pos_desired, uint16_t color=WHITE, float motor_time = motor_time,float motor_div = 1,bool goBack=true,bool lastSequence=true,bool showScreen=true);

void home_menu_screen(int array_size,const char *menu_name ,const char *const string_table[], uint16_t color=DEEPPINK);
int get_HomeMenu_Update(int s);

void configuration_menu_screen(int array_size,const char *menu_name ,const char *const string_table[],uint16_t color=DEEPPINK);
int get_Configuration_Menu_Update(int s);

void cameraSetting_menu_screen(int array_size,const char *menu_name ,const char *const string_table[],uint16_t color=DEEPPINK);
int get_CameraSetting_Menu_update(int s);

void positioning_menu_screen(int array_size,const char *menu_name ,const char *const string_table[],uint16_t color=DEEPPINK);
int get_positioning_Menu_update(int s);

void motor_calibration_menu1_screen(int array_size,const char *menu_name ,const char *const string_table[],uint16_t color=DEEPPINK);
int get_motor_calibration_menu1_update(int s);

void options_menu1_screen(int array_size,const char *menu_name ,const char *const string_table[],uint16_t color=DEEPPINK);
int get_options_menu1_update(int s);

void caliMenu(const char *const string_table[], int current_step, int max_steps=200, uint16_t color=WHITE, bool updateBar=false);
int calibrate(const char *const string_table[], int upper_limit, int lower_limit, uint16_t color=DEEPPINK);

void excess_menu_screen(int array_size,const char *menu_name ,const char *const string_table[],uint16_t color=DEEPPINK);
int get_excess_menu_update(int s);

void action_menu1_screen(int array_size,const char *menu_name ,const char *const string_table[],uint16_t color=DEEPPINK);
int get_Action_screen_1_Menu_update(int s);

void lens_movements_menu1_screen(int array_size,const char *menu_name ,const char *const string_table[],uint16_t color=DEEPPINK);
int get_lens_movements_menu1_update(int s);

void lens_movements_menu2_screen(int array_size,const char *menu_name ,const char *const string_table[],uint16_t color=DEEPPINK);
int get_lens_movements_menu2_update(int s);

void custome_movements_menu1_screen(int array_size,const char *menu_name ,const char *const string_table[], uint16_t color=DEEPPINK);
int get_custom_movements_menu1_update(int s);

void custome_movements_menu2_screen(int array_size,const char *menu_name ,const char *const string_table[], uint16_t color=DEEPPINK);
int get_custom_movements_menu2_update(int s);

void open_Shutter();
void close_Shutter();


void open_Shutter() { // Controls the shutter of a Nikon camera
  //Serial.println("Open");
  digitalWrite(FOCUS_CAMERA, HIGH);  
  digitalWrite(SHUTTER_CAMERA, HIGH);
  delay(150);
  digitalWrite(FOCUS_CAMERA, LOW);
  digitalWrite(SHUTTER_CAMERA, LOW);
  delay(150);
  digitalWrite(FOCUS_CAMERA, HIGH);  
  digitalWrite(SHUTTER_CAMERA, HIGH);
  delay(1000);
}
void close_Shutter() { // Controls the shutter of a Nikon camera
  //Serial.println("Close");
  digitalWrite(FOCUS_CAMERA, HIGH);  
  digitalWrite(SHUTTER_CAMERA, HIGH);
  delay(150);
  digitalWrite(FOCUS_CAMERA, LOW);
  digitalWrite(SHUTTER_CAMERA, LOW);
  delay(150);
  digitalWrite(FOCUS_CAMERA, HIGH);  
  digitalWrite(SHUTTER_CAMERA, HIGH);
  delay(1000);
}


void setup() {
  // put your setup code here, to run once:
  
  //for print statement
  Serial.begin(9600);
  EEPROM.begin(512);

  //for jp8900-16pin
  SoftwareSerial.begin(9600);

  //camera shutter and focus for capturing pictures 
  pinMode(FOCUS_CAMERA, OUTPUT);
  pinMode(SHUTTER_CAMERA, OUTPUT);
  digitalWrite(FOCUS_CAMERA, HIGH);  
  digitalWrite(SHUTTER_CAMERA, HIGH);
  delay(150);

  // ***** Joystick and Buttons *****
  pinMode(UP_BUTTON, INPUT_PULLUP);
  pinMode(DOWN_BUTTON, INPUT_PULLUP);
  pinMode(LEFT_BUTTON, INPUT_PULLUP);
  pinMode(RIGHT_BUTTON, INPUT_PULLUP);
  pinMode(CENTER_BUTTON, INPUT_PULLUP);
  pinMode(A_BUTTON, INPUT_PULLUP);
  pinMode(B_BUTTON, INPUT_PULLUP);
  pinMode(X_BUTTON, INPUT_PULLUP);
  pinMode(Y_BUTTON, INPUT_PULLUP);

  // ***** Motor *****
  main_motor.setMaxSpeed(RPM);
  // ***** Display *****
  tft.init(240, 240);
  tft.setRotation(3);
  tft.fillScreen(ST77XX_BLACK);//clear screen
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_WHITE);

  // ***** EEPROM Read *****
  // reads the stored memory
  lens_range = EEPROM.read(1);
  lens_current = EEPROM.read(3);
  orientation = EEPROM.read(4);
  shutter_time = EEPROM.read(5);
  motor_time = EEPROM.read(6);
  excess_option_set = EEPROM.read(7);

  //set back last know position after on/off
  setAccel(CALI_ACCEL);
  setCurrentPos(lens_current * MS_STEP);

  // ***** Default Values *****
  // if empty (==255), setting default values to 0
  // for current positions -> Move the motor to stored current 
  // to be implemented when finalise...

  //Show ICM APP 'loading page'
  initializing_Page();


}

void loop() {
  switch (home_screen) {

    //camera setting
    case 0: {
      //configuration menu
      switch (configuration_screen) {
        //camera_setting
        case 0: {
          switch (camera_setting_screen) {
            // Set Shutter Time screen - shutter time of the DSLR camera
            case 0:{
                //display shutter speed bar (motor calibration).
                // int max_shutter_time = 40;
                int old_shutter_time = shutter_time; //if go_home before setting
                hotbar(shutter_menu, shutter_time, max_shutter_time, true,0,1);
                while(true){
                  hotbar(shutter_menu, shutter_time, max_shutter_time, true, 0, 1, GOLDENROD, true);
                  int option = get_camera_calibration_update();
                  shutter_time = getLeftRight_value(max_shutter_time, shutter_time,0, 0);

                  //go home not set value
                  if(option == 0){
                    //reset to home inside get_camera_calibration_update();
                    shutter_time = old_shutter_time;
                    camera_setting_screen = -1;
                    break;
                  }
                  else if (option == 1){
                    EEPROM.write(5, shutter_time);
                    EEPROM.commit();
                    camera_setting_screen = -1;
                    break;
                  }
                }
                break;
            } 
            // set motor movement time -  time needed to execute a sequence
            case 1: {
              int motor_time_max = shutter_time;
              int old_motor_time = motor_time;
               hotbar(motor_time_menu, motor_time, motor_time_max, true,0,1);
              while(true){
                hotbar(motor_time_menu, motor_time, motor_time_max, true,0,1, GOLDENROD, true);
                int option = get_camera_calibration_update();
                motor_time = getLeftRight_value(motor_time_max, motor_time,0, 0);

                //go home not set value
                if(option == 0){
                  //reset to home inside get_camera_calibration_update();
                  motor_time = old_motor_time;
                  camera_setting_screen = -1;
                  break;
                }

                //Set value
                else if(option == 1){
                  EEPROM.write(6, motor_time);
                  EEPROM.commit();
                  camera_setting_screen = -1;
                  break;
                }
              }
              break;
            }
            //Excess Setting
            case 2: {
                  switch(excess_option_screen){
                    //pre
                    case 0:{
                      excess_option_set = 0;
                      excess_option_screen = -1;
                      //go back to prev screen after selection
                      camera_setting_screen = -1;
                      EEPROM.write(7, excess_option_set);
                      EEPROM.commit();
                      break;
                    }
                    //split
                    case 1:{
                      excess_option_set = 1;
                      excess_option_screen = -1;
                      //go back to prev screen after selection
                      camera_setting_screen = -1;
                      EEPROM.write(7, excess_option_set);
                      EEPROM.commit();
                      break;
                    }
                    //after
                    case 2:{
                      excess_option_set = 2;
                      excess_option_screen = -1;
                      //go back to prev screen after selection
                      camera_setting_screen = -1;
                      EEPROM.write(7, excess_option_set);
                      EEPROM.commit();
                      break;
                    }
                    default:
                      excess_menu_screen(3,excess_option_name,excess_option_menu,DEEPPINK);
                      excess_option_screen = get_excess_menu_update(excess_option_screen);
                      break;
                  }
              break;
            }
            //show [camera settings menu]
            default:
              cameraSetting_menu_screen(3,cs_name ,camera_settings_menu,DEEPPINK);
              camera_setting_screen = get_CameraSetting_Menu_update(camera_setting_screen);
              break;
          }
          break;
        }
        //motor callibration
        case 1: {
          switch (motor_calibration_screen1){
            //lens Calibration
            case 0: {
                lens_current = 0;
                setAccel(CALI_ACCEL);
                setCurrentPos(lens_current * MS_STEP);
            
                // set to minimum left
                int lens_min = calibrate(calilens_left, MOTOR_STEPS, -MOTOR_STEPS, DEEPPINK);
                setCurrentPos(0); // set to 0
                lens_current = 0;
                //updateScreen(100);
                
                // set to maximum right
                int lens_max = calibrate(calilens_right, MOTOR_STEPS, 0, DEEPPINK);
                lens_current = 0;
                lens_range = lens_max - lens_current;
                //updateScreen(100);
                EEPROM.write(1, lens_range);  

                //move and set lens current to be the midlle of the minMax(range)
                int lens_middle = lens_range / 2;
                lens_current = lens_middle;
                moveMotor(lens_middle,0);

                //minimum becomes absolute min pos
                EEPROM.write(3, lens_current);
                EEPROM.commit();
                motor_calibration_screen1 = -1;
                break;
            }
            //POV Calibration
            case 1: {
                setAccel(CALI_ACCEL);
                setCurrentPos(lens_current * MS_STEP);
                lens_current = chooseDist(3, lens_adjust, false, DEEPPINK);
                EEPROM.write(3, lens_current);
                motor_calibration_screen1 = resetScreen(motor_calibration_screen1);
                EEPROM.commit();
              break;
            }
            //Show motor_calibration_menu1
            default:
              motor_calibration_menu1_screen(2,mc1_name,motor_calibration_menu1,DEEPPINK);
              motor_calibration_screen1 = get_motor_calibration_menu1_update(motor_calibration_screen1);
              break;
          }
          break;
        }
        //options screen
        case 2: {
          switch(options_menu1){
            //Switch rotation
            case 0: {
              switch (camera_positioning_screen) {
                // len rotate 1-way
                case 0:{
                  orientation = 0;
                  EEPROM.write(4,orientation);
                  EEPROM.commit();
                  camera_positioning_screen = -1;
                  //go back to prev screen after selection
                  options_menu1 = -1;
                  break;
                }
                // len rotate other-way
                case 1:{
                  orientation = 1;
                  EEPROM.write(4,orientation);
                  EEPROM.commit();
                  camera_positioning_screen = -1;
                  //go back to prev screen after selection
                  options_menu1 = -1;
                  break;                
                }
                // show [positioning settings menu]
                default: 
                  positioning_menu_screen(2,pm_name ,positioning_menu,DEEPPINK);
                  camera_positioning_screen = get_positioning_Menu_update(camera_positioning_screen);
                  break;
              }      
              break;
            }
            //reset camera setting
            case 1:{
              EEPROM.write(5,0);
              EEPROM.write(6,0);                 
              EEPROM.write(7,0);
              shutter_time = 0;
              motor_time = 0;
              excess_option_set = 0;
              EEPROM.commit();
              options_menu1 = -1;
              break;
            }
            //reset calibration
            case 2:{
              EEPROM.write(1,0);
              EEPROM.write(3,0);
              EEPROM.write(4,0);
              lens_range = 0;
              lens_current = 0;
              orientation = 0;
              EEPROM.commit();
              options_menu1 = -1;
              break;
            }
            default:
              options_menu1_screen(3,option_name,options_menu,DEEPPINK);
              options_menu1 = get_options_menu1_update(options_menu1);
              break; 
          }
          break;
        }
        
        //Show configuation_menu
        default:
          configuration_menu_screen(3,mm_configuration_header,main_menu_1,DEEPPINK);  
          configuration_screen = get_Configuration_Menu_Update(configuration_screen);
          break;
      }
      break;
    }

    //Actions ZF
    case 1:{
      switch(action_screen_1){      
        //lens movement menu
        case 0: {
          switch(lens_movements_menu1){
            //lens to max 
            case 0:{
              Serial.println("lens to max");
              countdownMenu();
              //return to starting position by default
              goDist(lens_m1_0, lens_range, SNOW, motor_time,1,true,true,true);
              lens_movements_menu1 = resetScreen(lens_movements_menu1);
              break;
            }
            //lens to min 
            case 1:{
                Serial.println("lens to min");
                countdownMenu();
                //global motor time pass in by default
                //return to starting position by default
                goDist(lens_m1_1, 0, SNOW, motor_time,1,true,true,true);
                lens_movements_menu1 = resetScreen(lens_movements_menu1);
                break;
            }
            //lens to max and back 
            case 2:{
                    Serial.println("lens to max and back");                    
                    // Serial.print("previous_pos");
                    // Serial.println(previous_pos);
                    int previous_pos = lens_current;
                    countdownMenu();
                    //going back is now part of motor_time
                    goDist(lens_m1_2, lens_range, SNOW, motor_time,2,false,false,true);
                    goDist(lens_m1_2, previous_pos, SNOW, motor_time,2,false,true,false);
                    lens_movements_menu1 = resetScreen(lens_movements_menu1);
                    break;
            }
            //lens to min and back
            case 3:{
                    Serial.println("lens to min and back");                    
                    // Serial.print("previous_pos");
                    // Serial.println(previous_pos);
                    int previous_pos = lens_current;
                    countdownMenu();
                    //going back is now part of motor_time
                    goDist(lens_m1_3, 0, SNOW, motor_time,2,false,false,true);
                    goDist(lens_m1_3, previous_pos, SNOW, motor_time,2,false,true,false);
                    lens_movements_menu1 = resetScreen(lens_movements_menu1);
                    break;
            }
            //lens_movements_menu2
            case 4: {
              switch (lens_movements_menu2) {
                
                //lens to value 
                case 0:{
                Serial.println("lens to a value");
                int pos_desired;
                //choose dist and reset back to starting pos
                pos_desired = chooseDist(3, lens_dist, true, YELLOWGREEN);
                updateScreen();
                countdownMenu();
                //return to starting position by default
                goDist(lens_m2_0, pos_desired, YELLOWGREEN, motor_time,1,true,true,true);
                lens_movements_menu2 = resetScreen(lens_movements_menu2);
                break;
                }

                //lens to value and back
                case 1:{
                    Serial.println("lens to a value and back");
                    // Serial.print("previous_pos");
                    // Serial.println(previous_pos);
                    int pos_desired;
                    int previous_pos = lens_current;

                    //choose dist and reset back to starting pos
                    pos_desired = chooseDist(3, lens_dist, true, YELLOWGREEN);
                    updateScreen(100);
                    countdownMenu();

                    //going back is now part of motor_time
                    goDist(lens_m2_0, pos_desired, SNOW, motor_time,2,false,false,true);
                    goDist(lens_m2_1, previous_pos, SNOW, motor_time,2,false,true,false);
                    lens_movements_menu2 = resetScreen(lens_movements_menu2);
                    break;
                }

                default:
                   lens_movements_menu2_screen(2,lens_m2_name ,lens_menu2,DEEPPINK);
                   lens_movements_menu2 = get_lens_movements_menu2_update(lens_movements_menu2);
                   if(lens_movements_menu2 == -2){
                    lens_movements_menu1 = -1;
                    lens_movements_menu2 = -1;
                   }

              }
              break;
            }

            default:
              lens_movements_menu1_screen(4,lens_m1_name ,lens_menu1,DEEPPINK);
              lens_movements_menu1 = get_lens_movements_menu1_update(lens_movements_menu1);
          }
          break;
        }
        default:
          action_menu1_screen(1,mm_action1_header,main_menu_2,DEEPPINK);
          action_screen_1 = get_Action_screen_1_Menu_update(action_screen_1);
          break;
      }
      break;
    }

    //Action Custom
    case 2:{
      switch (fixed_paterns_menu1) {
        // Bokeh Effect (S: Focus Max, Zoom Current. F: Focus Current, Zoom Widest)
        // case 0: {
        //   Serial.println("Bokeh Effect");
        //   int previous_pos = focus_current;
        //   // setting lens to starting position
        //   printMoveSteps(-1, preset1_0, CADETBLUE, 2); 
        //   //moving motor, haven start pattern yet so use default motor speed
        //   moveMotor(FOCUS, focus_range, 0);
        //   focus_current = focus_range;
        //   //start pattern sequence
        //   updateScreen(100);
        //   countdownMenu();
        //   goDist(FOCUS, preset1_0, previous_pos, VIOLET, motor_time,2,false,false,true);
        //   goDist(ZOOM, preset1_0, 0, VIOLET, motor_time,2,true,true,false);
        //   fixed_paterns_menu1 = resetScreen(fixed_paterns_menu1);
        //   break;
        // }
        // //Firework Effect (Focus Max, then min, then return to original)
        // case 1: {
        //   Serial.println("Firework Effect");
        //   int previous_pos = focus_current;
        //   // setting lens to starting position
        //   printMoveSteps(-1, preset1_1, CADETBLUE, 2);
        //   //moving motor, haven start pattern yet so use default motor speed 
        //   moveMotor(FOCUS, focus_range,0);
        //   focus_current = focus_range;
        //   updateScreen(100);
        //   countdownMenu();
        //   // goDist(FOCUS, preset1_1, 0, AZURE, ((float)3/4)*motor_time, false,false,true);
        //   // goDist(FOCUS, preset1_1, previous_pos, AZURE, ((float)1/4)*motor_time,false,true,false);
        //   goDist(FOCUS, preset1_1, 0, AZURE, motor_time, 1.33,false,false,true);
        //   goDist(FOCUS, preset1_1, previous_pos, AZURE,motor_time,4,false,true,false);
        //   fixed_paterns_menu1 = resetScreen(fixed_paterns_menu1);
        //   break;
        // }
        // // Zoom Blur Effect (Focus & Zoom Max then back to original)
        // // Is going back to original part of motor time or not?
        // case 2: {
        //   Serial.println("ZoomBlur Effect");
        //   int previous_zoom_pos = zoom_current;
        //   int previous_focus_pos = focus_current;
        //   // setting lens to starting position
        //   printMoveSteps(-1, preset1_2, CADETBLUE, 2); // setting lens to starting position
        //   //asume going back part of motor time
        //   updateScreen(100);
        //   countdownMenu();
        //   goMultiDist(preset1_2, zoom_range, focus_range, VIOLET, motor_time,2, false, false,true);
        //   goMultiDist(preset1_2, previous_zoom_pos, previous_focus_pos, VIOLET, motor_time,2, false, true,false);
        //   fixed_paterns_menu1 = resetScreen(fixed_paterns_menu1);
        //   break;
        // }
        // //Sine Wave Effect
        // case 3: {
        //   Serial.println("Sine Wave Effect");
        //   int previous_zoom = zoom_current;
        //   int previous_focus = focus_current;
        //   // Serial.print("zoom_current");
        //   // Serial.println(zoom_current);
        //   // Serial.print("zoom_current");
        //   // Serial.println(zoom_current);
        //   countdownMenu();
        //   // goDist(ZOOM, preset2_0, zoom_range, CORAL, motor_time/4, false,false,true);
        //   // goDist(FOCUS, preset2_0, focus_range, CORAL, motor_time/4, false,false,false);
        //   // goDist(ZOOM, preset2_0, 0, CORAL, motor_time/4,false,false,false);
        //   // goDist(FOCUS, preset2_0, 0, CORAL, motor_time/4,false,true,false);
        //   goDist(ZOOM, preset2_0, zoom_range, CORAL, motor_time,4, false,false,true);
        //   goDist(FOCUS, preset2_0, focus_range, CORAL, motor_time,4, false,false,false);
        //   goDist(ZOOM, preset2_0, 0, CORAL, motor_time,4,false,false,false);
        //   goDist(FOCUS, preset2_0, 0, CORAL, motor_time,4,false,true,false);
        //   //end of pattern
          
        //   // return to initial position
        //   updateScreen(100);
        //   printMoveSteps(1, preset2_0, CADETBLUE, 1); 
        //   // Serial.print("previous_zoom");
        //   // Serial.println(previous_zoom);
        //   // Serial.print("previous_focus");
        //   // Serial.println(previous_focus);
        //   moveMultiMotor(previous_zoom,previous_focus,0);
        //   updateScreen(100);
        //   zoom_current = previous_zoom;
        //   focus_current = previous_focus;
        //   fixed_paterns_menu1 = resetScreen(fixed_paterns_menu1);
        //   break;
        // }

        // //fixed_paterns_menu2
        // case 4: {
        //   switch (fixed_paterns_menu2) {     
        //     //ZigZag
        //     case 0: {
        //       fixed_paterns_menu2 = resetScreen(fixed_paterns_menu2);
        //       break;
        //     }
            
        //     //show fixed_paterns_menu2
        //     default:
        //       custome_movements_menu2_screen(1, preset2_name, preset2_menu,DEEPPINK);
        //       fixed_paterns_menu2 = get_custom_movements_menu2_update(fixed_paterns_menu2);
        //       if(fixed_paterns_menu2 == -2){
        //             fixed_paterns_menu1 = -1;
        //             fixed_paterns_menu2 = -1;
        //       }
        //       break;        
        //   }
        //   break;
        // }

        //show fixed_paterns_menu1
        default:
          custome_movements_menu1_screen(4, preset1_name, preset1_menu,DEEPPINK);
          fixed_paterns_menu1 = get_custom_movements_menu1_update(fixed_paterns_menu1);
          break;
      }
      break;
    }

    //Home page
    default:
        home_menu_screen(3,home_header,home_menu,DEEPPINK);  
        home_screen = get_HomeMenu_Update(home_screen);
        break;
  }

}