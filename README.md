# Welcome to My Tar
***

<p float="left">
<img src="https://cdn-icons-png.flaticon.com/512/29/29575.png" width="150"> 

</p>


## Task
To recreate the basic functionaly of the tar command

## Description
The tar command has several options which change its functionality:
1. Take input files and create a .tar archive. (-c)
2. Append input files to an existing .tar archive. (-u)
3. Read out out files inside a .tar archive. (-t)
4. Extract files out of a given .tar arvhice. (-x)

In order to implement the functions, I created several functions that can 
produce a linked list of applicable file node. Each node contains a header struct
which contains the necessary file information (see [GNU Documentation](https://www.gnu.org/software/tar/manual/html_node/Standard.html))
as well as the data of the file. I make these linked lists constructable from given file names as well as from an existing tar archive.
Once the linked list is constructed, applying the above functions becomes more manageable.



## Installation
A Makefile is included which cleans and compile the program. Once can run ```make re``` to clean and compile.

## Usage
From the terminal 
```
./my_tar [-cutxf] filename.tar filename1, filename2, ...
```

### The Core Team


<span><i>Made at <a href='https://qwasar.io'>Qwasar SV -- Software Engineering School</a></i></span>
<span><img alt='Qwasar SV -- Software Engineering School's Logo' src='https://storage.googleapis.com/qwasar-public/qwasar-logo_50x50.png' width='20px'></span>
