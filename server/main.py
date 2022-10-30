import socket
import clientThread
import dataBases
HEADER_SIZE=23


def creatServerSocket():
    dataBases.createDataBase()
    HOST = "127.0.0.1"
    try:
        f = open("port.info", 'r')
        PORT = int(f.read())
        f.close()
    except:
        print("Error readeing PORT from file , defualt PORT is 1234")
        PORT = 1234
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((HOST, PORT))
        while True:
            s.listen()
            print(f"Server running on port {PORT} Listening to connections")
            conn, addr = s.accept()
            newThread=clientThread.ClientThread(addr,conn)
            newThread.run()

if __name__ == "__main__":
    creatServerSocket()
