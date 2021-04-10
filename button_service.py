from gpiozero import Button
from time import sleep
from signal import pause
import requests

def push_pedal(idx):
    def inner():
        print "Pressed " + str(idx)
        requests.get("http://localhost/push_button/" + str(idx))
    return inner

button_map = {
    21: 0,
    20: 1,
    16: 2,
    12: 3,
    6: 4
}

buttons = []
for gpio_pin, pedal_index in button_map.iteritems():
    button = Button(gpio_pin ,pull_up = True, bounce_time=0.002)
    button.when_pressed = push_pedal(pedal_index)
    buttons.append(button)

pause()
