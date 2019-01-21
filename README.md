The Arduino code behind our hardware hack for the 5th Annual IdeaHacks, taking place at UCLA.

Pertaining to the theme of transportation, we created a glove device aimed to provide safety for bikers, or enthusiasts of other vehicles, such as BIRDs, when riding at night.

This glove uses four flex sensors (pointer to pinky) to interpret certain hand gestures as left and right signals, to turn on our blinking lights (LED strips), that would be 
strapped onto the rider's backpack in our case. The LED strips are also used as braking lights, when the vehicle brakes and/or travels at slow speeeds. Also, apart from these 
safety signal light features, the glove uses a gyroscope to control the motor speed as one rotates their hand back and forth. All functionalities are realized through an Arduino 
Nano microcontroller programmed with the .ino file in this repository.


All in all, here is a more organized breakdown/list of the features offered:

Throttle Mode (Default)
- gradual start up from rest to desired motor speed (based on hand position)
- rotate hand back/up to speed up, and forth/down to slow down 
  - 70 degrees from flat backward to achieve max speed
  - 70 degrees from flat forward to achieve zero speed
- slowing down enough (60 degrees from flat backward) will cause brake lights to turn on

Signal Mode
- rotating hand back and forth makes no change to motor speed
- left blinker will be activated by lifting pointer and middle fingers
- right blinker will be activated by lifting middle, ring and pinky fingers

Coast Mode
- rotating hand back and forth makes no change to motor speed

To change between modes:
- from Throttle mode
  - to achieve Signal mode, raise pointer finger twice in quick succession
  - to achieve Coast mode, raise pointer finger thrice in quick succession
- from Coast or Signal mode
  - to achieve Throttle mode, raise pointer finger twice in quick succession
- no method to change between Coast and Signal modes directly# Project-Power-Glove
