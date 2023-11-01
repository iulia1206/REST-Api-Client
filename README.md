# Homework 4 Communication Protocols
- I implemented all tasks
- This implementation is based on lab 9 implementation
- I started by identifying the command and treated each specific case
- for building the JSON pattern I used the parson library from the homework's resources           
- I tested invalid inputs and printed an error message for each of them. If the input was invalid, I did not send the command to the server
- I used Postman to check the commands with an already implemented client
- for invalid input I parsed the string and checked to according limitations
- I also created an additional command **help** to print all possible valid commands

## Application flow:
1. Get the command from the user
2. Parse the command and treat invalid input
3. If additional inputs are necessary, show prompt for the user to receive them
5. Create server connection
6. Send at HOST:PORT/PATH **GET** and **DELETE** requests, the payload under format *application/json* for **POST** requests
7. Inform the user of the server's response