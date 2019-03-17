# --
a replacement for cat that will analyse tags and data in files

If -- is used to display a directory, e.g. -- ., then it will list the contents, directories, then links, then files.
Currently, these items are not in order. The listadditem() function should be extended to sort by name.

If -- is used to display a file, e.g. -- file.txt, then it will display the contents of the given files.
-- -R[]Y would make each character display as red, and [tags] to be displayed, with each tag in yellow.
Currently, there are no optimisations to streamline repeated colours, but this would be a useful feature.
Soon, -- will read unicode characters. Each will be replaced with ' ' with the character to be drawn on a bitmap overlay.
Soon, -- . will allow the directory to be explored as a menu, so that subdirectories and files can be opened.
Running the . command should redirect to -- .
