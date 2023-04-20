# Client-Server-Hangman-in-C
Client/Server Hangman in C with sockets libraries

To proper use the program you have to dowload the whole repository. After that you have to make the executable files of cliente.c and servidor.c by using
the Makefile.

Once you have the both executables you have to execute servidor.c in your terminal introducing the IP and port number also. After the server is created it will ask
to the user how many players he wants to play with and the 10 words to guess.

When you are done doing this you have to open as many termianls as players you chose before and execute cliente.c introducing the same IP and port number as before.
After all this you are ready to play your online multiplayer hangman.

All players share lives and after the lives are empty or the players win the game they will be requested if they want to play another time. If all players says
yes another game will be performed. If not all players are kicked out from the server and the server closes itself.
