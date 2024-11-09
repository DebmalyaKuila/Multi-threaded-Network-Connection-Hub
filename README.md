# Multi-threaded NetComm Hub
### a multi-threaded and multi-user network connection hub for group chat built in C++ 

### How to use ?

    1.Get inside the server directory , build and run serverSocket.cpp file 
        -If binding and listening is successful , the server will run on localhost at port:2000
        -If it fails , then check whether a process is till running on port:2000 
        
    2.Get inside client directory, build executable and create multiple instances of clients to simulate a multi-user group chat application    
    (Make sure you create multiple instances from your linux terminal , not from IDE terminal as IDE terminals will kill the previous client process 
    and create a new client process , so you won't be able to use multi-user functionality)
        -In the client terminal ,at first type your username
        -Now, Just type away your message and hit enter, your message will be broadcasted to all connected users
        -To exit a chat , just write "exit()" and hit enter
## Note :

    - This code can only be run in Linux Machine