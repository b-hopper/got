# Got Version Control System

### Overview

Got is a rudimentary version control system which served as a great
opportunity for our team to learn several system's programming
techniques such as signal handling, asynchronous I/O, etc.

**Contributors** : Brad Hopper, Bradley Thompson, Marie Escapita


### License

This program is released under the "MIT Lecense".
Please refer to the file **LICENSE** for more information.


### How to use

Must be compiled with '-lrt' flag
"cc got.c -lrt"

* To initialize a got repository, type "./got init"
  1. This will create your .gotdir and setup the config file within.
* To stage files you'd like to commit to a version directory within 
  .gotdir, use "./got add [file1] [file2] ..."
* To view the version for the project within the current working directory,
  as well as the files staged to be committed, type "./got status"
* To store those files in .gotdir (within the newest version directory),
  type "./got commit"
* To pull stored files from past project versions, type "./got reset [desired_version_num]"
