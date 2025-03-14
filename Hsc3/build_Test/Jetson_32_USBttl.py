import serial
import time
# import struct
 
try:
    # 初始化串口
    serial_port = serial.Serial(
        port="/dev/ttyUSB0",
        baudrate=115200,
        bytesize=serial.EIGHTBITS,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
    )
    time.sleep(1)  # 等待串口连接稳定
 
    # # 发送int数据
    # int_value = 1234
    # int_bytes = struct.pack('>I', int_value)  # 大端字节序
    # serial_port.write(int_bytes)
    # serial_port.write(';'.encode())
 
    # 接收int数据 
    # if serial_port.in_waiting >= 4:  # int数据大小为4字节
    #     int_received_bytes = serial_port.read(4)
    #     int_received = struct.unpack('>I', int_received_bytes)[0]
    #     print(f"接收到的int数据: {int_received}")
 
    while 1:
        # # 发送char数据
        # char_value = 'A'
        # serial_port.write(char_value.encode())
        # serial_port.write(';'.encode())
 
        # 发送字符串数据
        str = "ABCD"
        serial_port.write(str.encode())
        serial_port.write(';'.encode())
 
        # 接收char数据
        if serial_port.in_waiting > 0:
            # char_received = serial_port.read().decode()
            # print(f"接收到的char数据: {char_received}")
            char_byte = serial_port.read()  # 读取一个字节
            char_str = char_byte.decode()  # 将字节转换为字符
            print(f"接收到的字符: {char_str}")
 
            if char_str == "A":  # 字符比较 复位回消息可能会产生问题
                print("已接收字符 A")
 
        if serial_port.inWaiting() > 0:
            data = serial_port.readline().strip()
            print(data)
            if data == b"ABCD":  # 注意这里是字节比较
                print("已接收ABCD")
 
 
except serial.SerialException as e:
    print(f"串口错误: {e}")
except Exception as e:
    print(f"其他错误: {e}")
finally:
    if serial_port.is_open:
        serial_port.close()