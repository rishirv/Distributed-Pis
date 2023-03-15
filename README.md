# Distritbuted-Pis


Ben's Version of shit as he does weird developing things that a person who knew what they were doing would totally do because he totally knows what he is doing .... don't question it. 

#########################################################################
NOTE: To use this you must first do a couple things to direct all the makefiles to this repo's libraries (and not to your 140e class libs). 

First open up your .zsh file (or whatever the windows/linux alternative is). For me this looks like: vim ~/.zshrc 

Now you should see a line: 
export CS140E_2023_PATH="....

Youll want to copy this line and rename it to
exportCS140E_PROJ_PATH="..../Distributed-Pis"

Where the .... is the path up to the /Distributed-Pis folder. (which will depend on where you have the folder cloned to)

okay now you can save that, and then run the command: source ~/.zshrc
to refresh and make the changes visible. 
(or whatever your config file for your terminal is if its not zsh) 

Now you should be able to succesfully run make. If for some reason you get an error of the sort (could not find rule for ... ) it likely means you have the incorrect path in the .zsh file. 
#########################################################################


Connect GPIO pin 21 to D6 on ESP8266
Connect GPIO pin 23 to D5 on ESP8266


Other-Stuff: All the stuff that was in main before I started messing with it. 
Projects: Basically like 'labs' in class repo except project pieces, each folder is a feature being developed. 
Libpi: same as before, but slowly changing headers/adding appropriate sources when each part is complete. 
