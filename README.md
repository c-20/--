# --
a replacement for cat that will analyse tags and data in files

If -- is used to display a directory, e.g. -- ., then it will list the contents, directories, then links, then files.
Currently, these items are not in order. The listadditem() function should be extended to sort by name.

If -- is used to display a file, e.g. -- file.txt, then it will display the contents of the given files.

-- -R[]Y would make each character display as red, and [tags] to be displayed, with each tag in yellow.

Currently, line tags are hidden by default. Inline tags are ignored. -I enables inline tag matching (no \n required).

Displaying tags is disabled with '-()' (the default). To ignore line tags, a -l/-L option will be added.

Currently, there are no optimisations to streamline repeated colours, but this would be a useful feature.

Soon, -- will read unicode characters. Each will be replaced with ' ' with the character to be drawn on a bitmap overlay.

-- will also read data blocks and links to data files as single character blocks with the bitmap to be drawn on the overlay. The overlay data can also specify a number of rows and columns to occupy (including 0x0 for a dot, 0x1 for a vertical bar, and 1x0 for a horizontal bar), plus behaviours for the surrounding characters. Inline images will shift the chars under to the right, which would either adjust overflow positioning (should words be broken?). Underlay images will shift one to the left for the 0x0. -12.5,0+0x0 will shift 12.5 characters (12 * 10 + 5 pixels for block width 10) to the left.

^^ update: -- now reads data blocks that begin with \[\\0tag\\0\] and end with \[/tag\\0\]

There are many parameters that have not yet been given input options

Soon, -- . will allow the directory to be explored as a menu, so that subdirectories and files can be opened.

Later, running the . command should redirect to -- .

Running the -- command could open stdin, in the same way as cat.

Or, -- could open -- . and allow a new file to be created if <Enter> is pressed. Down down enter might run nano, if nano is the default command associated with a particular file type.
