Information:

If you have used RE firmware before, and updates with DFU you might run into a issue with pin-conflicts.

We dont use FSIO in our config, but use much simpler Gp-Pwm maps instead.

So if you had pins set for FSIO and run into this issue, simply use the included "Unbrick_Basetune.msq" to clear the pin-settings.

You can also hand-edit this in your tune to continue using it. 