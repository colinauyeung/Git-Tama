import serial
import time
import requests

#writes to the serial port to communicate with the arduino
def sendToArduino(sendStr):
  testSend = (sendStr).encode()
  ser.write(testSend)

#Recieves data from the arduino
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

# wait until the Arduino sends 'Arduino Ready' - allows time for Arduino reset
# it also ensures that any bytes left over from a previous message are discarded
def waitForArduino():

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

#Send the commit data recieved from the github api to the arduino
#sends the username of the commiter and the total number of changes in the commit
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


#Which port the arduino is connected
serPort = '/dev/cu.usbmodem14501'
baudRate = 9600

#start and end markers for serial reading from the arduino
startMarker = 60
endMarker = 62




#the url of the api for the event list of a specific repo
#details: https://developer.github.com/v3/activity/events/
repo_url = "https://api.github.com/repos/colinauyeung/GOLD/events"

#authentication token for accessing the github api
#details: https://developer.github.com/v3/#authentication
token = "insert your own Github access token here"

#the resource tag (eTag) that can be grabbed by the reponse header after accessing
#the repo from the api the first time
#details: https://developer.github.com/v3/#rate-limiting
eTag = '"(insert eTag from API here)"'
headers = {"Authorization": "Bearer (insert your own Github access token here)", "If-None-Match": eTag}

#the last pushevent idea, so that you're not accessing the same commit twice
last_id = ""

while(True):
    #get from the github api
    response = requests.get(repo_url, headers=headers)

    #if the resource has not been changed continue
    if response.status_code == 304:
        continue

    #if the resource has been successfully accessed
    if response.status_code == 200:

        #grab the information
        package = response.json()

        #scan through the event list
        for i in range(0,30):

            #if an event is a PushEvent
            if package[i]["type"] == 'PushEvent':

                #check to make sure that it isn't one already responsed to
                if package[i]["id"] != last_id:

                    #grab the commit url from the PushEvent
                    commiturl = package[i]["payload"]["commits"][0]["url"]

                    #Access the commit information directly
                    response_commit = requests.get(commiturl, headers=headers)
                    if response_commit.status_code == 200:
                        package_commit = response_commit.json()

                        #Grab the commit user and total changes
                        size = package_commit["stats"]["total"]
                        user = package_commit["committer"]["login"]
                        print(size)

                        ## Open a serial connection to the arduino
                        ser = serial.Serial(serPort, baudRate)

                        #Sent the commit data to the arduino
                        sendCommit(user,size)

                        #close the connection
                        ser.close()

                    last_id = package[i]["id"]
                break


    time.sleep(2)
