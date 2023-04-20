# Client-Server-Hangman-in-C
Client/Server Hangman in C with sockets libraries


Hemos implementado los objetivos extra 1,2 y 4. Para el primer objetivo hemos creado con símbolos los diferentes estados que puede tomar el "ahorcado"
con respecto a las vidas que les quedan a los jugadores para posteriormente printearselos.
Para el segundo objetivo solicitamos al usuario en el servidor que elija el número de jugadores que va a tener la partida. Para esto hemos creado un for
que a medida que avanza la partida le asigna a cada jugador su turno y que dentro tiene más bucles for que infoman de cada acción a los jugadores.
Para el último objetivo realizado hemos creado una tabla de booleanos que almacena al final de la partida si cada jugador quiere continuar y un booleano que
almacena si se va a volver a jugar o no. En el cliente hemos implementado que si quiere volver a jugar espere una respuesta del servidor que va a ser
una nueva partida o el final de la partida en función de si el resto de jugadores quieren continuar o no.
