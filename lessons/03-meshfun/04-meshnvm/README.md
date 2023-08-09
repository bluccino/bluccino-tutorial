# 04-meshnvm (using WLSTD wireless core, HWTINY hardware core and NVM memory)

## Sample Description

* mesh application which can count system starts and stores a current @ix in NVM
* utilizes non-volatile memory (NVM)
* every system start the number of total system starts are displayed
* pressing button @1 causes two things
  1) the next LED pair is selected for pair wise blinking (pair number @ix
     is stored in NVM)
  2) a remote LED controlled by GOOCLI @1 is toggled
