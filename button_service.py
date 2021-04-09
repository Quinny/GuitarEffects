from gpiozero import Button
from time import sleep
from signal import pause
import requests

def push_pedal(idx):
    def inner():
        print "Pressed " + str(idx)
        requests.get("http://localhost/push_button/" + str(idx))
    return inner

button = Button(21 ,pull_up = True, bounce_time=0.002)
button.when_pressed = push_pedal(0)

pause()
