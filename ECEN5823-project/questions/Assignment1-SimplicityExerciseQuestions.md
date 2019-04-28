Please include your answers to the questions below with your submission, entering into the space below each question
See [Mastering Markdown](https://guides.github.com/features/mastering-markdown/) for github markdown formatting if desired.

**1. How much current does a single LED draw when the output drive is set to "Strong" with the original code?**

Single LED draw 4.96mA current when the output drive is set to "Strong".

**2. After commenting out the standard output drive and uncommenting "Weak" drive, how much current does a single LED draw?**

After commenting out the standard output drive and uncommenting "Weak" drive, it draws 4.90mA current

**3. Is there a meaningful difference in current between the answers for question 1 and 2? Please explain your answer, 
referencing the [Mainboard Schematic](https://www.silabs.com/documents/public/schematic-files/WSTK-Main-BRD4001A-A01-schematic.pdf) and [AEM Accuracy](https://www.silabs.com/documents/login/user-guides/ug279-brd4104a-user-guide.pdf) section of the user's guide where appropriate.**

There is no meaningfull difference in current between the answers for Question 1 and 2.
Because the value of resistors used before both the LEDS (1 and 0) are the same. The number of resistors used are also the same.
(Information taken from the 2 manuals given in the question). Thus there wont be any significant difference between a strong and a weak drive.

**4. Using the Energy Profiler with "weak" drive LEDs, what is the average current and energy measured with only LED1 turning on in the main loop?**

Average Current when LED 1 is on - 4.76mA
Energy when LED 1 is on - 12.11uWh

**5. Using the Energy Profiler with "weak" drive LEDs, what is the average current and energy measured with both LED1 and LED0 turning on in the main loop?**

Average current when LED1 and LED0 are on - 4.93mA
Energy when LED1 and LED0 are on - 12.52uWh
