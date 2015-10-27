# CmdLineMath

So this will currently work on OS X only.
The CmdLine args package is in CmdArgs and 
should be built/installed so that this 
exec can link it. 


Once CmdArgs is built, you can run 
```bash
make
```
and you'll get the exe. 
You can set quite a few parameters from the
command line. To write a log of all the 
output use the -log xxx name. The extension
will be a .csv but you can change it if you
like within the source code.

To exit, hit ctrl-c

Here's the current options:
```bash
*******************************************************************************
Options menu for: ./simplemath
-add                          // Include addition problems, 
                                 default: true, mandatory: false 
-sub                          // Include subtraction problems, 
                                 default: false, mandatory: false 
-mult                         // Include multiplication 
                                 problems, default: false, mandatory: false 
-number                       // Set total number of problems, 
                                 default: 20, mandatory: false 
-frac_success                 // Set fraction .xx to get 
                                 correct before exiting, default: 0.750, 
                                 mandatory: false 
-min                          // Set minimum digit to use, 
                                 default: -9, mandatory: false 
-max                          // Set maximum digit to use, 
                                 default: 9, mandatory: false 
-log                          // Set a log file, .csv extension 
                                 added by default, default: , mandatory: 
                                 false 
-h                            // Print menu and exit, default: 
                                 true, mandatory: false 
End Options
*******************************************************************************
```

Default arg operation:
```bash
CmdLineMath>>./simplemath
What does: -2 + -5 = -7
-7 is correct, GOOD JOB!!
What does: 1 + -6 = -5
-5 is correct, GOOD JOB!!
What does: -7 + 9 = ^C
Your % correct is: 100
```

That doesn't capture what got my son interested though, which was the sound. The OS X <i>say</i> command speaks the equations and the result (good or bad). There's also a logging feature since my son has to turn in some math work each weak that the parents are required to come up with. 

Disclaimer: I wrote this in about 10 minutes so the code/organization is a bit messy. I'll clean up in time. Feel free to submit requests, or fixes in the interim. One thing that bugs me is the way <i>say</i> says "x - -x" as "x minus minus x" instead of "x minus negative x" which can be fixed, just need to type a few more lines of code.
```
