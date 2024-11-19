import serial
baudrates = [9600, 19200, 115200]
parities = [serial.PARITY_NONE, serial.PARITY_EVEN, serial.PARITY_ODD]

def read_block(ser_conn, num_lines):
    for line in range(num_lines):
        print(ser_conn.readline())

def query(ser_conn, message):
    ser_conn.write((message + '\r').encode()) 
    read_block(ser_conn, 10) 


"""
for baudrate_for_loop in baudrates:
    for parity_for_loop in parities:
        print(baudrate_for_loop, parity_for_loop)
        serial_conn = serial.Serial('COM3', baudrate=baudrate_for_loop, parity=parity_for_loop, timeout=0.1)
        # Hit the enable (EN) on the ESP32 before this read_block finishes
        read_block(serial_conn, 50)
        serial_conn.close()
exit()  
"""
# To connect to the board and input the password:
serial_conn = serial.Serial('COM3', baudrate=19200, parity=serial.PARITY_EVEN, timeout=0.1)
read_block(serial_conn, 100)  
passwd = 'I0t' 
query(serial_conn, passwd) 
query(serial_conn, "wifi_info") 
serial_conn.close() 
