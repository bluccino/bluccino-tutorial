// toggle.b

  (A) := app;  (D) := bl_down;  (U) := bl_up;

  (U) -> [BUTTON:PRESS @ix] -> (A)
      -> {
           // onoff[@ix] = ! onoff[@ix];
         }
      -> [LED:SET @ix,onoff[@ix]] -> (D);
