# 06-meshnode (using WLSTD wireless core, HWTINY hardware core and NVM memory)

* augmentation of sample 05-nvm by mesh node house keeping functions
* mesh application which can count system starts
* utilizes non-volatile memory (NVM)
* every system start another LED blinking pair is being selected
* pressing button @1 turns the blinking state on or off (toggle)
* in addition house keeping is done:
  a) quick status LED blinking if un-provisioned, slow blinking if provisioned
	b) periodic blinking of main LED and status LED off if attentioning
	c) node reset possibility based on reset counter logic
  d) node reset possibility based on button press during startup sequence
