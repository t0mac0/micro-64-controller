# Introduction #

When I originally wanted to start playing with the N64 controller protocol I used the only PIC I had on hand which was a 16f628a. Despite being my first real microcontroller project and killing many PICs I managed to create a functioning controller.

# Details #

The Controller works by constantly updating its inputs and an interrupt triggers the controller to read the data line for communication. The console initiates all communication so the approach works quite well although it would be impossible with even a slightly lower clock speed.

The two largest constraints with working with the 16f628a were the number of inputs and speed of the micro. The 20Mhz is internally divided by 4 to get a clock speed of 5 cycles a microsecond. Considering that the controller bit bangs a signal with a 1us resolution of sorts then there aren't any instruction cycles to waste. The N64 console sends 4 distinct commands of which 2 are implemented. The status and button request are the only commands which are needed for the vast majority of games.

When using a crystal and dedicating one input to the data line there are just enough inputs to to implement the most commonly used buttons on the N64 controller. These are Start, A, B, R, Z and the C buttons. Of course the joystick is kinda good as well.

After working out a fairly functional PIC based controller I wanted a good way to show off my fairly uninteresting breadboard project. Creating the smallest N64 controller ever seemed like a good idea to show off the controller's ability. I put together the controller over the course of a weekend for a competition on the Benheck forums. The triggers are mapped to R and Z and to save space on the controller the C buttons were replaced with a 4 directional tact switch.

As you can see the directional tact doesn't have its button face on it. I had one C button from a dead controller which I dropped behind my desk while I was building the controller and I never found it again.

![http://i297.photobucket.com/albums/mm207/evilteddy_2008/Controllerinhands-1.png](http://i297.photobucket.com/albums/mm207/evilteddy_2008/Controllerinhands-1.png)

![http://i297.photobucket.com/albums/mm207/evilteddy_2008/Controllerwithconsole-1.png](http://i297.photobucket.com/albums/mm207/evilteddy_2008/Controllerwithconsole-1.png)

The controller worked fairly well but as with nearly all projects there are improvements that I'd like to make. These include improving the how the joystick is handled, adding macros, an option to remap buttons, more joysticks (like a C stick similar to the gamecube) and implementing the rumble and memory pack.

Rather than continue with the limited hardware the next iteration of the N64 controller will use a faster micro with more inputs and enough memory to implement all of these functions.

[Here's](http://www.youtube.com/watch?v=YT_C8aPI8m8) a short video of the controller in action.