//#define ZEPHYR_RUN
//#define LINUX_RUN

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef LINUX_RUN
  #include <math.h>
#endif

#ifdef ZEPHYR_RUN
  #include <zephyr/random/random.h>
  //#include <include/sys/math.h>
#endif

#define INIT_TEMPERATURE 28
#define INIT_PRESSURE 102325
#define INIT_HUMIDITY 70
#define INIT_LUMINANCE 1

#define GOAL_TEMPERATURE 25
#define GOAL_PRESSURE 101325
#define GOAL_HUMIDITY 50
#define GOAL_LUMINANCE 150

#define DRIFT_CONST 1.01

#define MSG_BUF_MAX_SIZE 1024
#define DRIFT_TIME 10*60                       // Require 10 mins to reach
                                               // desirable effect of each sensor

#define INCREASE_CONST 0.05                    // If the actuator is turned off
                                               // The value would increase 0.05 per sec

extern char send_buf[MSG_BUF_MAX_SIZE];
extern char recv_buf_client[MSG_BUF_MAX_SIZE];

extern enum sensor_type{
  TEMPERATURE,
  PRESSURE,
  HUMIDITY,
  LUMINANCE,
  SENSOR_LENGTH
} sensor_type;

extern enum act_type{
  AIRCON,
  VENTILATION,
  HUMIDIFIER,
  LIGHT,
  DIM_LIGHT,
  ACTUATOR_LENGTH
} act_type;

typedef struct sensor{
  enum sensor_type sensorType;
  float value;
} sensor_t;

typedef struct actuator{
  enum act_type actType;
  int working;
  int workingTime;
  int downTime;
} actuator_t;

/* DEFINE SET OF ACTUATORS */
extern actuator_t air_con;

extern actuator_t vent_fan;

extern actuator_t hum;

extern actuator_t light;

extern actuator_t dim_light;
/* END OF DEFINING ACTUATORS */

/* DEFINE EVERY SENSORS POSSIBLE */
extern sensor_t * sensor_list;

/* END DEFINE SENSORS */

/* DEFINE FUNCTIONS */
void init_sensor();

void update_sensor(char* sensor, float value);
 
void clean_up_function();

int process_recv(char *buf);
int process_reply(char *buf);