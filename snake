from sense_hat import SenseHat
import random
from time import sleep

# Initialize Sense HAT and game variables
sense = SenseHat()
speed = 0.3
snake = [(3, 3)]  # Starting position of the snake, initially with one segment
direction = (1, 0)  # Initial movement direction (right)
score = 0

# Define colors
w = (0, 0, 0)  # Background color (black)
g = (0, 255, 0)  # Snake color (green)
r = (255, 0, 0)  # Item color (red)

# Place the first item on the screen at a random location
item = (random.randint(0, 7), random.randint(0, 7))
while item in snake:
    item = (random.randint(0, 7), random.randint(0, 7))


def draw_snake():
    """Clear the screen and draw the snake and item."""
    sense.clear()
    for segment in snake:
        sense.set_pixel(segment[1], segment[0], g)  # Set each segment to green
    sense.set_pixel(item[1], item[0], r)  # Set the item to red


def move_snake():
    """Calculate the snake's new head position based on the current direction."""
    global item, score, speed
    new_head = (snake[0][0] + direction[0], snake[0][1] + direction[1])

    # Check for collision with walls or self
    if (
        new_head[0] < 0 or new_head[0] > 7 or
        new_head[1] < 0 or new_head[1] > 7 or
        new_head in snake
    ):
        sense.show_message("Game Over!", scroll_speed=0.05, text_colour=r)
        sense.show_message("Score: " + str(score), scroll_speed=0.05, text_colour=g)
        return False  # End game

    # Check if snake eats the item
    if new_head == item:
        score += 1
        snake.insert(0, new_head)  # Grow the snake
        # Place a new item randomly
        item = (random.randint(0, 7), random.randint(0, 7))
        while item in snake:
            item = (random.randint(0, 7), random.randint(0, 7))
        
        # Increase speed slightly
        speed = max(0.05, speed - 0.01)  # Cap the speed so it doesn't get too fast
    else:
        # Move the snake by adding the new head and removing the tail
        snake.insert(0, new_head)
        snake.pop()

    return True


def joystick_movement(event):
    """Update the direction based on joystick input."""
    global direction
    if event.action == 'pressed':
        if event.direction == 'up' and direction != (1, 0):
            direction = (-1, 0)
        elif event.direction == 'down' and direction != (-1, 0):
            direction = (1, 0)
        elif event.direction == 'left' and direction != (0, 1):
            direction = (0, -1)
        elif event.direction == 'right' and direction != (0, -1):
            direction = (0, 1)


# Bind joystick events
sense.stick.direction_up = joystick_movement
sense.stick.direction_down = joystick_movement
sense.stick.direction_left = joystick_movement
sense.stick.direction_right = joystick_movement

# Game loop
while True:
    draw_snake()
    if not move_snake():
        break  # Game over
    sleep(speed)