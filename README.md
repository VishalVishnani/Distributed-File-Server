ECEN 5273- Network Systems

DISTRIBUTED FILE SERVER

Author: Vishal Vishnani

Date: 11/19/2017

Description: A Distributed File System is a client/server based application that allows client to store and retrieve files on multiple servers. One of the features of Distributed file system is that  each  file  can  be  divided  in  to  pieces  and  stored  on  different  servers and  can  be retrieved even if one server is not active.

File Structure:

1]udp_client.c - Client takes the command from user, sends it to the server and then waits for server's response.
                 The list of command includes:
                 -get [file_name] [subfolder name]
                 -put [file_name] [subfolder name]
                 -ls  [subfolder name]

2]udp_server.c - The server responds to the client's request.

3]Makefile - Makefile to create executable


Execution:

1]First create executable and start the server.
    make server
    ./server [port number]

2]Now start the client.
    make client
    ./client 

3]Now the client can take command from user and send it to server.
