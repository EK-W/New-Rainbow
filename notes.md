# TO DO:
- ~~Write an implementation in rainbowFactory.~~
- ~~Separate it into multiple files.~~
- ~~Separate headers into folder~~
- ~~Allow setting of starting points~~
- ~~Consider moving back to working with coords instead of pixels~~
- Decouple Display from rest of rainbow.
- ~~Create functions to aid setting up configuration structs~~
- Make function naming schemes more like SDL, always beginning with their "class" type.
- Create utility file with code to, for instance, set pixel colors and stuff
- Standardize field naming conventions
- Add informational functions (like getting the size of the queue, for example)
- Make a struct rainbowSetup which includes function pointers to each of the customizable generation functions
	- Make there be default rainbowSetup functions and then functions to change each function.
	- Hey uhhhhh how are we gonna deal with struct definitions??
- ~~Create a default functions folder~~
- Figure out how to allow users to specify parameters for ColorBrain (and other things, probably) even though the parameters needed vary between implementations.
	- This shouldn't be super difficult. Just have the users specify the parameters using custom functions defined in the implementation files. Or something.
- Implement the cube sublist tree thing to speed up the color finding
- Separate arguments into structs
- Add color transformation stuff to the display function
- Overhaul documentation
- Make error messages more descriptive
- Figure out how to only send SDL_Quit when everything is done.
- Write as many guarantees as possible
- Consider inlining certain functions
- Make a separate malloc for each thing being allocated instead of everything in a class getting allocated at the same time
- To address modulo bias for random numbers, make a utility class that generates random numbers for us, and figure out modulo bias
	there. This would also allow us to handle the (unlikely) situation where the maximum desired value is greater than RAND_MAX. 
- Create a logging utility class to allow controls for how verbose the program is.
- Change boolean function names to better reflect the fact that they're booleans.
- Figure out if for some reason the wrong coordinates are being added to the generation queue.
- Address SDL messing up the random number gen.
- Randomly choose pixels which will be excluded from generation until the end, thus dispersing the buggy pixels generated at the end.


# Thoughts:
- closest available color should not necessarily be customizable, as the closest color available *should* always return the same color, regardless of implementation (except in the case that multiple colors are equally distant from the desired color). Perhaps it would make sense to rename the function "ideal available color", as that would allow different implementations to decide for themselves which colors are more ideal than others when the preferred color is not available.

# Thoughts about color and dimensions:
- The fact that this arranges colors is not actually that important. Right now, we are programming for three dimensions (r, g, and b) but in theory the number of dimensions is sorta arbitrary. Making this work for any number of dimensions could allow for some cool results. And I'm not sure that anything would conceptually be very different. The only problematic part would be figuring out how to represent the "color" values correctly.
- If we were to implement this in this more general way, there would be two things to think about:
	- 1) Bits per dimension
	- 2) Number of dimensions



## ChooseNextPixelCoord
- Really, there's not much opportunity for customization here. The customization comes more from adding the coordinates to the queue.
- Maybe the Queue should have some sort of priority system!

## GetColorPointAtCoord
- Customizing this will change how getPreferredColor interacts with space. Lots of opportunity for strange things.
- Could create color-wormholes
- Could make the edges of the screen wrap around

# GetPreferredColor
- New pixel chance of completely changing target color.
- Customizable radii
- Weight one direction higher than other directions

## GetIdealAllowedColor
### Options for GetIdealAllowedColor:
- Emptying list method
	- Slow
- Octant-sphere method
- cube-to-sphere method
	- is this one possible?
	- Check out 3DVis2.scad for diagrams.
- Emptying cube sublists
	- This can work together with the other methods.
	- This is a good idea.
- A cube-tree structure
	- Very good idea.

### Ideas for GetIdealAllowedColor:
- Weigh differences in hue differently than differences in saturation and value
- Make it search for colors that are slightly different instead of as close as possible