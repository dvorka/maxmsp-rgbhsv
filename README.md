MAX MSP RGB/HSV
===============

MAX MSP external object for RGB/HSV conversion.


INTERFACE
=========

Input

*   [Sensor R G B] ... message
*   [White_Saturation_From White_Saturation_To ...Black... ...Gray... ] ... message

Output

*   [Sensor H S V]
*   [Sensor Color_ID]

Predefined colors (Hue)

*   #1      Red      0-21    
*   #2      Orange   22-38
*   #3               39-62

Format

*   Hue            0..359
*   Saturation     0..100
*   Value          0..100
*   Red            0..255
*   Green          0..255
*   Value          0..255


BUGS
----
https://github.com/dvorka/macmsp-rgbhsv/issues

