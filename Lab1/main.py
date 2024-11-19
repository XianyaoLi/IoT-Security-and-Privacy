import serial
baudrates = [9600, 19200, 115200]
parities = [serial.PARITY_NONE, serial.PARITY_EVEN, serial.PARITY_ODD]

def read_block(ser_conn, num_lines):
    var = ""
    for line in range(num_lines):
        var += ser_conn.readline().decode("utf-8", errors='ignore')
    return var

def query(ser_conn, message):
    ser_conn.write(message.encode())
    return read_block(ser_conn, 10)

# for baudrate_for_loop in baudrates:
#     for parity_for_loop in parities:
#         print(baudrate_for_loop, parity_for_loop)
#         serial_conn = serial.Serial('COM3', baudrate=baudrate_for_loop, parity=parity_for_loop, timeout=0.3)
#         read_block(serial_conn, 30)
#         serial_conn.close()

serial_conn = serial.Serial('COM3', baudrate=19200, parity=serial.PARITY_EVEN, timeout=0.1)
read_block(serial_conn, 50)
nums = ['0', '1', '2', '3', '4', '5', '6', '7', '8', '9']
upper_letters = ['A', 'B', 'C', 'D', 'E', 'F', 'G', 'H','I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z']
lower_letters = ['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z']
break_out_flag = False
for first_char in upper_letters:
    for second_char in nums:
        for third_char in lower_letters:
            if(break_out_flag): 
                break
            else:
                passwd = first_char + second_char + third_char + '\r'
                message = query(serial_conn, passwd)
                print(passwd)
                print(message)
                if "[error]" in message:
                    pass
                else:
                    break_out_flag = True