import threading
import  Request
import  Response
HEADER_SIZE=23


#creating new client thread , get the socket and client address
class ClientThread(threading.Thread):
    def __init__(self, clientAddress, clientsocket):
        threading.Thread.__init__(self)
        self.csocket = clientsocket
        print("New connection added: ", clientAddress)
        self.clientAddress=clientAddress




#run the session with the client , listen to his requests untill the end of session
    def run(self):
        try:
            print("Connection from : ", self.clientAddress)
            while True:
                data = self.csocket.recv(HEADER_SIZE)
                req = Request.RequestHeader()
                req.unpackRequestHeader(data, self.csocket)
                # if crc fail sent we need to recive new file request , we jump to recv again to listen socket
                if req.code == Request.REQUEST_CODES["ERRCRC"]:
                    print("Waiting for new request file")
                    #we contionue and keep listen to client for resend the file
                    continue
                res = Response.ResponseHeader()
                reply = res.packResponse(req)
                if  req.code==Request.REQUEST_CODES["ERR4CRC"] or res.code == Response.RESPONSE_CODES["MESSEGE_SUCCSESS"]  :
                    #end session with client
                    self.csocket.close()
                    print("Client at ", self.clientAddress, " disconnected...")
                    break
                self.csocket.sendall(reply)
        except:
            print("Error connection with  client ")





