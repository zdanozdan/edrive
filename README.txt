How to connect external stepper drive to controller board.

1. Open collector configuration

Connect 2-wire cable from stepper drive to CON4 on controler board. Positive voltage
from stepper drive (pulse input) should be connected to pin marked as PULSE on PCB
controller board. Stepper GND should be connected to pin marked as P- on PCB.

2. Internal pull-up configuration

If stepper drive needs to be pulled up, jumper JP1 should be closed to '12V pull up'
position as marked on PCB. This configuration will provide 12V (or other voltage
accordingly to used transformer) feed through R19 (1 KOhm) resistor. When CON4
disconnected you should be able to measure this voltage between PULSE and P- pins on
CON4.
If this configuration is used there may be need for closing JP2 if controller
board GND and stepper drive GND are separated. 

3. External pull-up configuration

If stepper drive provides it's own pull-up voltege this can be used after setting JP1
to 'Ext pull up' as marked on PCB. External pull-up voltege should be then connected
between P+ and P- on CON4. There is additional pull-up resistor R50 (1 KOhm). This
configuration is the much more noise resistant than this in point 2, as both system
are still separated.


RECOMENDATIONS

Use configuration 1 as it's the simplest possible, supported by most drives. If
pull-up necessery use rather 3 and feed external pull up voltege from stepper drive
if available.


How to configure virus option.

Go to 'Maintenance setup' menu. Before setting up the virus make sure you have set up
correct date and time. Here you have 2 possibilities : password '013579' 
gives you full admin access over the system and has to be used to set up virus
option (never disclosure it to customer) . Go to PM menu (short from PAY ME). System
will let you in as opposite to using '474705' passoword. In PM menu you will have 3
options : Activation - where you set up activation date, Password - where 4 digit
disarm code is entered and Status which shows if virus option is enabled or disabled.

Try to set up this same date as current date of the system. Virus status should
change to ENABLED. As soon as the system is shut down and power up again system will
ask you for 4 digit pin code you entered in 'password' menu. Entering valid code will
disarm virus option. Notice that nobody can change current date and time now.

CONSIDERATIONS.
Real time clock, on which virus option is based, is backed up with external battery
which is used when system is shut down. If virus will be enabled , the battery CANNOT
be removed when main power is off. If you remove the battery the system will
interpret it as 'hacking' and ask you for pin code at startup. However this may cause
the problems if during instaliation or operation somebody accidentely removes the
battery. 

I strongly recommed putting some kind of maybe red sticker on the battery in
virus-enabled systems.


PROBLEMS HISTORY: 

###########################################################################

ENTRY 13/10/2005
Problem with pulling resistor

Hi Paul,

I think I've found the problem.

1. The troublemaker is OP1 optocoupler, on previous batch it was marked as
 816C now is 816L. I've checked on datasheet and they differs in some
parameters. Funny thing is that they just sell 816 and you cannot choose
the type :-(

2. I've screw up some resistor calculations and unfortunatelly using 816C
it was working fine but with 816L it doesn't. Actually I think even using
816C we have been just on the edge. My mistake.

The solution :

1. To give OP1 more current we need to change R20 and R21 from 330 Ohm to
about 150 Ohm. I've checked it with internal pull up configuration and it
seems to be working fine on the table. However cannot test with motor as I
don't have a drive with external pull up.

2. R20 and R21 are 2 small SMT resistors on bottom layer. They are close
to keyboard connector (there are 6 SMT resistors in 2 rows). R20/R21 are
those 2 marked as 331 in right column (in SMT terminology it means 33 plus
one 0 =330 ).

3. The simplest solution is to make a sandwich SMT putting another 330 Ohm
on top of existing one. It's pretty easy to do and can be done with
standard soldering station and oridinar solder. Those 2 connected in
parallel will now have a resistance of 330*330/330+330 = 165 Ohm. If you
don't have SMT 330 Ohm resistor (I guess you don't) you may remove R20/R21
and solder normal 150 Ohm resistor to the pads - that's obiously for test
only. Alternativelly you can take 330 from other board. All systems with
external pull up configuration would have to be patched that way
unfortunatelly.

4. If you want me to come over and patch it for you just let me know.

Really sorry for that, unfortunatelly sometimes shit happens :-(

BR
Tom

#############################################################################

ENTRY 11/07/2006
1. Problem with displaying total count error when count goes over 1500. FIXED
2. Problem with pull lenght an hologram offset. Offset should be substracted from
total pull length as we do got overrrun. FIXED
3. Pull input should be disabled when holo alarm goes off. Holo search reenables
input. FIXED


############################################################################
ENTRY 11/01/2007

Problem with new version of SDCC compiler 2.6.0 #4309 (Jul 28 2006). Generated
code create warnings and keyboard doesn't read properly, also a lot of warrnings
generated during compilation.

Solution: Use last batch compiled code (holofix 1.hex) which works OK. Binaries
added to CVS
