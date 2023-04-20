todo: servidor.c cliente.c
    gcc servidor.c -o servidor
    gcc cliente.c -o cliente

borrar: servidor cliente
    rm cliente
    rm servidor