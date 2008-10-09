BUGS
====

- After editing library the auto-maximize is wrong...
- Flip still doesn't quite work... (try arcs/polygons)
- when editing a component placed on a diagram, the L,R,U,D,M 
  buttons don't seem to work from the keyboard - unless one of them is 
  already the current focus...

- The junctions are sometimes sent to the back layer and you can see wires crossing them. I don't 
  really know whether this is a bug or not, but I think it would be better if they were 
  automatically sent to the front and stayed there. Now I always have to send some to the 
  front one by one. 
  Also, if you zoom in close, the junctions don't seem to be exactly in the center. I have an image 
  of that at http://img179.echo.cx/img179/8624/junctions4aa.gif. The grid is set at "Fine grid spacing".


- It should be useful to add a checkbox "select all" libraries. The "select one library at time" 
  is OK. The edit line "search string" inside libraries works if at least one library has been 
  selected. Adding "select all" avoid to check all libraries after one "select one library at 
  time" command.



Library editor
==============

Design details for symbols
Show power pins for each symbol

- Pins look wrong in library when placed left and rotated to left...

- There is something strange with position of parts. When you choose 
  position 'down' numbers of pins are under the pins (example attached). 


Wish List
=========

- Is there a way to get the Design Detail information (Title, Author,
  etc.) and assign it to a component parameter?  I would like to do my own title block, 
  perhaps a symbol with no pins, and be able to have it pick up the Design Details data 
  automatically and display it. 

- For pins: Is there a way to define a non-proportional font? Or is it possible to specify 
	a fixed distance between the end of the text and the start of the pin?


Mouse:
- Option for scroll wheel to scroll the page
- Option for right-click to cancel the current option (like 1.60)

- Locate the labels on symbols correctly manually
- Show label names & values on symbols
- Multi-line text...
- Arrow keys to move object

- possibility of set project path / project functionality?
- "add wire" option to pins in context menu
- (optional) Arrow heads on lines

- True Arcs
- Change the colour/sizes of wires
- Change the layout of the design block etc..
- Different symbol body shapes for each part in package
- Highlight all wires on a net & show net list name/number

- Add a "jump" over unconnected wires

- Add a flexible zoom menu, including fit width, height & page
- Add a option to set scroll wheel sensitivity
- Group/Ungroup optons

Jacob:

- How about the option of printing one symbol per page with all attributes
  shown, including a standard title block and border? 

Darek:

- Could pin numbers be placed slightly lower (closer to the pin)? There are problems 
  when pins are every 2mm.

- The library with C167 is in example.mdb. It isn't so important for me but I wonder what 
  to do with 2xx pins parts for example. It isn't so stupid nowadays (it is normal pin 
  count for FPGA's). I've seen the solution in Eagle. A part can be devided into several 
  parts which can be moved independently from others (for example port P1, port P2...etc.)

- Show the current grid setting (fine etc..) on a toolbar



SPICE simulator "run button"

1. Generate the spice netlist. (Same name and directory as the schematic, just different extension.) 
2. Copy the SPICE Model property of the run node, (which I think is currently unused), to a batch file. (Same name and directory as the schematic, just different extension.) 
3. Set current directory to the directory were the schematic is. 
4. Run the batch file with the spice netlist as parameters. 


Wim

1. If I go to colours-power all I can change is the colour of the text near the power lines, the 
   lines themself always stay black..... not a big thing, but if you're gonna play with line 
   colours, you want to play with all the line colours.






FIXED:

- When I type "µF" (with the Greek mu) at a symbol, the "u" gets stuck to the "F". 
  To illustrate that, there's a picture of it at http://img241.echo.cx/img241/3730/uf2ui.gif.
