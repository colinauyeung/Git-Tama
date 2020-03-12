import serial
import time
import requests

def sendToArduino(sendStr):
  testSend = (sendStr).encode()
  ser.write(testSend)

def recvFromArduino():
    global startMarker, endMarker

    ck = ""
    x = "z" # any value that is not an end- or startMarker
    byteCount = -1 # to allow for the fact that the last increment will be one too many

  # wait for the start character
    while  ord(x) != startMarker:
        x = ser.read()

  # save data until the end marker is found
    while ord(x) != endMarker:
        if ord(x) != startMarker:
            ck = ck + x.decode(encoding='UTF-8')
            byteCount += 1
        x = ser.read()
    return(ck)


#============================

def waitForArduino():

   # wait until the Arduino sends 'Arduino Ready' - allows time for Arduino reset
   # it also ensures that any bytes left over from a previous message are discarded

    global startMarker, endMarker

    msg = ""
    while msg.find("Arduino is ready") == -1:

      while ser.inWaiting() == 0:
        pass

      msg = recvFromArduino()

      print (msg)


#init serial port and bound
# bound rate on two ports must be the same

def sendMessage(messageC):

    waitingForReply = False

    teststr = messageC

    if waitingForReply == False:
        sendToArduino(teststr)
        print ("Sent from PC -- LOOP NUM " + str(1) + " TEST STR " + teststr)
        waitingForReply = True

    if waitingForReply == True:

        while ser.inWaiting() == 0:
            pass

        dataRecvd = recvFromArduino()
        print ("Reply Received  " + dataRecvd)
        waitingForReply = False

        print ("===========")

        time.sleep(5)

def sendCommit(user,size):
    servoMode = 0
    if size < 10:
        servoMode = 1
    elif size < 100:
        servoMode = 2
    else:
        servoMode = 3
    userNum = 0
    if user == "colinauyeung" :
        userNum = 1
    else:
        userNum = 2
    #ser = serial.Serial(serPort, baudRate)
    #print ("Serial port " + serPort + " opened  Baudrate " + str(baudRate))
    waitForArduino()
    sendMessage("<"+ str(userNum) + "," + str(servoMode) + ">")



serPort = '/dev/cu.usbmodem14501'
baudRate = 9600
#ser = serial.Serial(serPort, baudRate)
#print ("Serial port " + serPort + " opened  Baudrate " + str(baudRate))


startMarker = 60
endMarker = 62

#waitForArduino()

#send data via serial port
#sendMessage("<2,1>")
#ser.close()



repo_url = "https://api.github.com/repos/colinauyeung/GOLD/events"
token = "insert your own Github access token here"
eTag = '"(insert eTag from API here)"'
headers = {"Authorization": "Bearer (insert your own Github access token here)", "If-None-Match": eTag}
last_id = ""

while(True):
    response = requests.get(repo_url, headers=headers)
    if response.status_code == 304:
        print(304)
        continue
    package = response.json()

    if response.status_code == 200:

        for i in range(0,30):
            if package[i]["type"] == 'PushEvent':
                if package[i]["id"] != last_id:
                    commiturl = package[i]["payload"]["commits"][0]["url"]
                    response_commit = requests.get(commiturl, headers=headers)
                    if response_commit.status_code == 200:
                        package_commit = response_commit.json()
                        size = package_commit["stats"]["total"]
                        user = package_commit["committer"]["login"]
                        print(size)

                        ## Serial output code would go here
                        ser = serial.Serial(serPort, baudRate)
                        sendCommit(user,size)
                        ser.close()

                    last_id = package[i]["id"]
                break


    time.sleep(2)
