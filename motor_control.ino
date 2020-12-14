
/*

+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  ACCELERATING      CONSTANT VELOCITY     DECELERATING
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
velocity
^
|                 _____________________
|               /                       \
|             /                           \
|           /                               \
|         /                                   \
|       /                                       \
|     /                                           \
|   /                                               \
|------------------------------------------------------------> t

*/


/*
                                ACCELERATION
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  ACCELERATING      CONSTANT VELOCITY       DECEL       -CONSTANT VELOCITY
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
velocity
^
|                 _____________________                                    
|               /                       \                                  
|             /                           \                                
|           /                               \                               
|---------/-----------------------------------\-------------------------------------> t
|       /                                       \                               /
|     /                                           \                            /
|   /                                               \ ______________________ /

*/



// INPUTS
#define step_pin        3   // drive pin:  1 (WHT) & 14 (BLU)
#define direction_pin   4   // drive pin:  2 (YEL) & 15 (ORG)
#define reset_pin       11  // drive pin: 11 (GRY) & 23 (LT BLU)
#define shutdown_pin    5   // drive pin: 16 (RED) & 17 (BRN)
#define fault_pin       6   // drive pin:  9 (GRN) & 21 (PUR)

#define estop_pin       2   // normally closed
#define go_pin          8   // go button (pin 2 & 3)
#define stop_pin        9   // stop button (pin 5 & 6)

// OUTPUTS
// #define fault_output

String incoming_string = "";
char incoming_ints[5];  // placeholder for inputted speed, should never be more than 1000 realistically

int starting_accel = 2000;  // constant, this should be way higher than any reasonable move
int speed = 100;  // DEFAULT SPEED, lower number = faster, this is the time in between pulses

// FLAGS
// default system startup states
bool shutdown_flag  = true;
// bool fault_status   = true;
// bool reset_flag     = false;
bool forward_flag   = false;
bool reverse_flag   = false;
bool acceled        = false;

void setup() {
    Serial.begin(115200);
    
    pinMode(reset_pin, OUTPUT);
    digitalWrite(reset_pin, LOW);   // default: not in reset state

    pinMode(shutdown_pin, OUTPUT);

    pinMode(fault_pin, INPUT);

    pinMode(step_pin, OUTPUT);

    pinMode(direction_pin, OUTPUT);
    digitalWrite(direction_pin, HIGH);  // default: forward

    pinMode(go_pin, INPUT_PULLUP);
    pinMode(stop_pin, INPUT_PULLUP);
    pinMode(estop_pin, INPUT_PULLUP);

}


void loop() {
    read_string();
    get_button_status();

    if (!shutdown_flag) {
        if (forward_flag || reverse_flag) { move(); }
    }
}


void move() {
    /*
    To move: send pulses to the step_pin to increment the motor forwards or backwards
    
    STEP PIN
        minimum step pulse: 200 nanoseconds (0.0002 milliseconds / 0.2 microseconds)
        minimum off time: 200 nanoseconds (0.0002 milliseconds / 0.2 microseconds)
    */

    if (acceled) {
        // if we have accelerated, constant velocity
        constant_velocity(speed);
    }
    // accelerating
    else if (!acceled) {
        // if we haven't accelerated yet, accelerate
        accelerate();
    }
}


void accelerate() {
    for (float i = starting_accel; i >= speed; i--) {
        // Serial.println(i);
        constant_velocity(i);

        if (i == speed) {
            // if we've reached velocity setpoint
            acceled = true;
        }
    }
}

void constant_velocity(int speed) {
    digitalWrite(step_pin, HIGH);
    delayMicroseconds(speed);
    digitalWrite(step_pin, LOW);
    acceled = true;
}


void get_button_status() {
    // read status of the stop & go buttons
    if (digitalRead(go_pin) == 0) { forward(); }
    if (digitalRead(stop_pin) == 0) { stop(); }
    if (digitalRead(stop_pin) == 0 && digitalRead(go_pin) == 0) { reset(); }    // for testing purposes, so you don't have to reset via serial monitor
    if (digitalRead(estop_pin) == 1) { off(); }
}

void read_string() {
    // read data from Serial
    if (Serial.available() > 0) {
        incoming_string = Serial.readString();
        incoming_string.toCharArray(incoming_ints, 5);

        if      (incoming_string == "f\r\n")        { forward(); }
        else if (incoming_string == "r\r\n")        { reverse(); }
        else if (incoming_string == "stop\r\n")     { stop(); }
        else if (incoming_string == "off\r\n")      { off(); }
        else if (incoming_string == "reset\r\n")    { reset(); }

        else if (isDigit(incoming_ints[0])) {
            speed = incoming_string.toInt();
            Serial.print("changed speed to: ");
            Serial.println(speed);
        }
    }
}

void stop() {
    Serial.println("STOP");
    // uncomment these after testing...they're just super annoying
    // digitalWrite(shutdown_pin, HIGH);
    // shutdown_flag = true;
    forward_flag = false;
    reverse_flag = false;
    acceled = false;
}

void reset() {
    Serial.println("RESET");
    digitalWrite(shutdown_pin, LOW);
    // reset_flag = true;
    shutdown_flag = false;
    forward_flag = false;
    reverse_flag = false;
    acceled = false;
}

void off() {
    Serial.println("OFF");
    digitalWrite(shutdown_pin, HIGH);
    // reset_flag = false;
    shutdown_flag = true;
    forward_flag = false;
    reverse_flag = false;
    acceled = false;
}

void forward() {
    Serial.println("FORWARD ------->");
    digitalWrite(direction_pin, HIGH);
    // reset_flag = false;
    forward_flag = true;
    reverse_flag = false;
    Serial.print("speed: "); Serial.println(speed);
}

void reverse() {
    Serial.println("REVERSE <-------");
    digitalWrite(direction_pin, LOW);
    // reset_flag = false;
    forward_flag = false;
    reverse_flag = true;
    Serial.print("speed: "); Serial.println(-speed);
}