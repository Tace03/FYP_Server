//#define ZEPHYR_RUN
//#define LINUX_RUN

/*
#define TEMP_SENSOR 1
#define PRES_SENSOR 0
#define HUM_SENSOR 1
#define LUM_SENSOR 1
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>

#ifdef LINUX_RUN
  #include <math.h>
#endif

#ifdef ZEPHYR_RUN
  #include <zephyr/random/random.h>
  //#include <include/sys/math.h>
#endif

#include "client_sensor.h"



// Initialize things here
char prs_time[7] = "";
char old_time[7] = "";
int delta_time;
char float_num[32];

time_t t;
struct tm *tmp;
char time_component[6];

int sensor_size = SENSOR_LENGTH;

actuator_t air_con = {
  .actType     = AIRCON,
  .working     = 0,
  .workingTime = 0,
  .downTime    = 0 
};

actuator_t vent_fan = {
  .actType = VENTILATION,
  .working = 0,
  .workingTime = 0,
  .downTime    = 0 
};

actuator_t hum = {
  .actType = HUMIDIFIER,
  .working = 0,
  .workingTime = 0,
  .downTime    = 0 
};

actuator_t light = {
  .actType = LIGHT,
  .working = 0,
  .workingTime = 0,
  .downTime    = 0 
};

actuator_t dim_light = {
  .actType = DIM_LIGHT,
  .working = 0,
  .workingTime = 0,
  .downTime    = 0 
};

sensor_t* sensor_list;
//support function
void init_sensor();

//visible functions to other files
void update_sensor(char* sensor, float value);

int process_recv(char*);
int process_reply(char*);

/////////////////// SECTION 1 /////////////////////////

/*
  Section 1
  Functionality: Derive init and clean up function for sensors
 */
void init_sensor(){
  
  sensor_list = (sensor_t*)malloc(sizeof(sensor_t) * SENSOR_LENGTH);
  
  printf("sensor_size : %d\n", sensor_size);
    
    
  sensor_list[TEMPERATURE].sensorType = TEMPERATURE; 
  sensor_list[TEMPERATURE].value = 0; 

  sensor_list[PRESSURE].sensorType = PRESSURE; 
  sensor_list[PRESSURE].value = 0;
      
  sensor_list[HUMIDITY].sensorType = HUMIDITY; 
  sensor_list[HUMIDITY].value = 0;
       
  sensor_list[LUMINANCE].sensorType = LUMINANCE; 
  sensor_list[LUMINANCE].value = 0;
    
  /*
  printf("%.2f\n", sensor_list[0].value);
  printf("%.2f\n", sensor_list[1].value);
  printf("%.2f\n", sensor_list[2].value);
  printf("%.2f\n", sensor_list[3].value);
  */
}

void clean_up_function(){
  free(sensor_list);
}

//////////////// END OF SECTION 1 /////////////////////////

//////////////// SECTION 2 ///////////////////////////////

int process_reply(char *buf){

  int ret = 0;
  char temp_time[2];
  time(&t);
  tmp = localtime(&t);
  
  strcpy(time_component, "");

  if(tmp->tm_hour < 10) sprintf(temp_time, "0%d", tmp->tm_hour);
  else sprintf(temp_time, "%d", tmp->tm_hour);
  strcat(time_component, temp_time);

  if(tmp->tm_min < 10) sprintf(temp_time, "0%d", tmp->tm_min);
  else sprintf(temp_time, "%d", tmp->tm_min);
  strcat(time_component, temp_time);

  if(tmp->tm_sec < 10) sprintf(temp_time, "0%d", tmp->tm_sec);
  else sprintf(temp_time, "%d", tmp->tm_sec);
  strcat(time_component, temp_time);

  // example message
  // {time:000001,air_con:1,vent_fan:0,hum:1,light:1,}
  strcpy(buf, "{");
  // add time
  
  strcat(buf, "time:");
  strcat(buf, time_component);
  strcat(buf, ",");

  // loop to insert the command
  for(int i = 0 ; i < SENSOR_LENGTH; i++){
    switch(sensor_list[i].sensorType){
      case TEMPERATURE:
        if(sensor_list[i].value == 0) break;
        strcat(buf, "air_con:1,");
        break;
      case PRESSURE:
        if(sensor_list[i].value == 0) break;
        strcat(buf, "vent_fan:1,");
        break;
      case HUMIDITY:
        if(sensor_list[i].value == 0) break;
        strcat(buf, "hum:1,");
        break;
      case LUMINANCE:
        if(sensor_list[i].value == 0) break;
        strcat(buf, "light:1,");
        break;
      default:
        break;
    }  
  }
  strcat(buf, "}");

  // DEBUG MESSAGE
  printf("buf equals: %s\n", buf);
  printf("length of buf equals : %d\n", sizeof(buf) - 1);
  
  return ret;
}

////////////// END OF SECTION 2 /////////////////

////////////// SECTION 3 ////////////////////////

int process_recv(char *buf){
  int ret = 0;


  if(buf[0] != '{'){
    printf("ERROR - Msg doesn't have opening brackets\n");
    return 0;
  }
  
  char temp_word[1024] = "";

  char cur_char;

  char sensor[1024] = "";
  float temp_value;

  int i = 1;

  // example of message from sensor
  // {temp:27.69,pres:0,humid:67.93,lum:150.00,}

  do{
    // valid check iterator
    if( i >= MSG_BUF_MAX_SIZE){
      printf("ERROR- Msg doesn't have closing bracket\n");
      return -1;
    }
    // get the keys or get the values
    // if get values then can use atoi or atof later
    cur_char = buf[i];

    // check strncat for use of this function
    if(cur_char != ':' && cur_char != ',') strncat(temp_word, &cur_char, 1);
    else{
      if(cur_char == ':') strcpy(sensor, temp_word);
      if(cur_char == ',') {
        // process the key-value pair and turn on/off the actuator

        temp_value = (float) atof(temp_word);
        update_sensor(sensor, temp_value); // trigger the value if the processed value is not time
        printf("Current temp_value: %f\n", temp_value);

      }
      // after taking in values, reset the buffer
      strcpy(temp_word, "");
    }

    // increment iterator
    i++;
  }
  while( buf[i] != '}');

  return ret;
}

void update_sensor(char *sensor, float value){
  // DEBUG MESSAGE
  //printf("%s \n", act);

  // example of message from sensor
  // {temp:27.69,pres:0,humid:67.93,lum:150.00,}
  if(!strcmp(sensor, "temp")){
    sensor_list[TEMPERATURE].value = value;
  }

  if(!strcmp(sensor, "pres")){
    sensor_list[PRESSURE].value = value;
  }

  if(!strcmp(sensor, "humid")){
    sensor_list[HUMIDITY].value = value;
  }

  if(!strcmp(sensor, "lum")){
    sensor_list[LUMINANCE].value = value;
  }
}
////////////// END OF SECTION 3 /////////////////





