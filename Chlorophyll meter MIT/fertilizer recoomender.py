import sys
import ibmiotf.application
import ibmiotf.device
import random
import json
import cv2
import numpy as np
import time
import RPi.GPIO as GPIO
import math
GPIO.setmode(GPIO.BOARD)
GPIO.setwarnings(False)
GPIO.setup(3, GPIO.IN)
import requests
#Provide your IBM Watson Device Credentials
organization = "cpdswc"
deviceType = "nodemcu"
deviceId = "1999"
authMethod = "token"
authToken = "123456789"

def myCommandCallback(cmd):
        sum1=0
        sum2=0
        sum3=0

        avg1 = 0
        avg2 = 0
        avg3 = 0
        avg4 = 0
        mylist1=[]
        mylist2=[]
        mylist3=[]
        count=1
        print("Command received: %s" % cmd.data['command'])


        if cmd.data['command']=='on':
                print('Button Pressed')
                #img1 = cv2.imread('crop1.jpg')
                cam = cv2.VideoCapture(0)
                s,img = cam.read()
                cv2.imwrite('crop' +str(count)+'.jpg',img)
                img1=cv2.imread('crop' +str(count)+'.jpg')
                for x in range(0,10):
                        for y in range(0,x):
                                px = img1[x,y]
                                blue = img1[x,y,0]
                                num1 = float(blue)
                                mylist1.append(num1)
                                #print mylist1
                                green = img1[x,y,1]
                                num2 = float(green)
                                #print num2
                                mylist2.append(num2)
                                #print mylist2 ==
                                red = img1[x,y,2]
                                num3 = float(red)
                                #print num3
                                mylist3.append(num3)
                                #print mylist3
                for i1 in mylist1:
                        sum1+=i1
                        avg1=sum1/len(mylist1)
                print (" avg percentage opf blue %s" %(avg1))
                for i2 in mylist2:
                    sum2+=i2
                    avg2=sum2/len(mylist2)
                print (" avg percentage opf green %s" %(avg2))       
                for i3 in mylist3:
                    sum3+=i3
                    avg3=sum3/len(mylist3)
                print (" avg percentage opf red %s"  %(avg3))
                avg4 = ((avg2-(avg3 + avg1) / float(2)))/255
                print (" avg of green %s"%(avg4))
                chlorophyllpercent = (1/(1 + math.exp(-avg4)))
                print ("normalized content of green %s" %(chlorophyllpercent))
                r=requests.get("https://ksr61.eu-gb.mybluemix.net/farmer?data="+str(chlorophyllpercent))
                print(r)
                #ser.write("#"+str(chlorophyllpercent)+"~")
                print ("done printing")
                cam.release()
                count=count+1
        state = 0

                

try:
	deviceOptions = {"org": organization, "type": deviceType, "id": deviceId, "auth-method": authMethod, "auth-token": authToken}
	deviceCli = ibmiotf.device.Client(deviceOptions)
	#..............................................
	
except Exception as e:
	print("Caught exception connecting device: %s" % str(e))
	sys.exit()

# Connect and send a datapoint "hello" with value "world" into the cloud as an event of type "greeting" 10 times
deviceCli.connect()

while True:

        time.sleep(1)
        
        deviceCli.commandCallback = myCommandCallback

        
