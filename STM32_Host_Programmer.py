import serial
import struct
import os
import sys
import glob
import crcmod
#import time
Flash_HAL_OK                                        = 0x00
Flash_HAL_ERROR                                     = 0x01
Flash_HAL_BUSY                                      = 0x02
Flash_HAL_TIMEOUT                                   = 0x03
Flash_HAL_INV_ADDR                                  = 0x04
Flash_BOOTLOADER_ADDR                               = 0x08000000
Flash_EXT_NEXT                                      = 0x05
Flash_WRITE_OK                                      = 0x06
Flash_WRITE_ERROR                                   = 0x07


#BL Commands
COMMAND_BL_GET_VER                                  = 0x51
COMMAND_BL_GET_HELP                                 = 0x52
COMMAND_BL_GET_CID                                  = 0x53
COMMAND_BL_GET_RDP_STATUS                           = 0x54
COMMAND_BL_GO_TO_ADDR                               = 0x55
COMMAND_BL_FLASH_ERASE                              = 0x56
COMMAND_BL_MEM_WRITE                                = 0x57
COMMAND_BL_EN_R_W_PROTECT                           = 0x58
COMMAND_BL_MEM_READ                                 = 0x59
COMMAND_BL_READ_SECTOR_P_STATUS                     = 0x5A
COMMAND_BL_OTP_READ                                 = 0x5B
COMMAND_BL_DIS_R_W_PROTECT                          = 0x5C
COMMAND_BL_GO_TO_BOOTLOADER                         = 0x5D
COMMAND_BL_EXT_MEM_WRITE                            = 0x5E
COMMAND_BL_EXT_MEM_TO_MEM_WRITE                     = 0x5F



#len details of the command
COMMAND_BL_GET_VER_LEN                              = 6
COMMAND_BL_GET_HELP_LEN                             = 6
COMMAND_BL_GET_CID_LEN                              = 6
COMMAND_BL_GET_RDP_STATUS_LEN                       = 6
COMMAND_BL_GO_TO_ADDR_LEN                           = 10
COMMAND_BL_FLASH_ERASE_LEN                          = 8
COMMAND_BL_MEM_WRITE_LEN                            = 11
COMMAND_BL_EXT_EM_WRITE_LEN                         = 11
COMMAND_BL_EN_R_W_PROTECT_LEN                       = 8
COMMAND_BL_READ_SECTOR_P_STATUS_LEN                 = 6
COMMAND_BL_DIS_R_W_PROTECT_LEN                      = 6
COMMAND_BL_MY_NEW_COMMAND_LEN                       = 8
COMMAND_BL_EXT_MEM_TO_MEM_WRITE_LEN                 = 6

verbose_mode = 1
mem_write_active =0
mem_ext_write_active =0


#----------------------------- file ops----------------------------------------

def calc_file_len():
    size = os.path.getsize("mySource_App.bin")
    return size

def open_the_file():
    global bin_file
    bin_file = open('mySource_App.bin','rb')
    #read = bin_file.read()
    #global file_contents = bytearray(read)

def read_the_file():
    pass

def close_the_file():
    bin_file.close()




#----------------------------- utilities----------------------------------------

def word_to_byte(addr, index , lowerfirst):
    value = (addr >> ( 8 * ( index -1)) & 0x000000FF )
    return value

def get_crc(buff, length):
    Crc = 0xFFFFFFFF
    #print(length)
    for data in buff[0:length]:
        Crc = Crc ^ data
        for i in range(32):
            if(Crc & 0x80000000):
                Crc = (Crc << 1) ^ 0x04C11DB7
            else:
                Crc = (Crc << 1)
    return Crc
def ethernet_crc_accumulate(data_list, limit, Crc=0xFFFFFFFF):
    polynomial = 0x04C11DB7
    
    for data in data_list[0:limit]:
        Crc = Crc ^ data
        for i in range(32):
            if(Crc & 0x80000000):
                Crc = (Crc << 1) ^ polynomial
            else:
                Crc = (Crc << 1)
    return Crc
    

#----------------------------- Serial Port ----------------------------------------
def serial_ports():
    """ Lists serial port names

        :raises EnvironmentError:
            On unsupported or unknown platforms
        :returns:
            A list of the serial ports available on the system
    """
    if sys.platform.startswith('win'):
        ports = ['COM%s' % (i + 1) for i in range(256)]
    elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
        # this excludes your current terminal "/dev/tty"
        ports = glob.glob('/dev/tty[A-Za-z]*')
    elif sys.platform.startswith('darwin'):
        ports = glob.glob('/dev/tty.*')
    else:
        raise EnvironmentError('Unsupported platform')

    result = []
    for port in ports:
        try:
            s = serial.Serial(port)
            s.close()
            result.append(port)
        except (OSError, serial.SerialException):
            pass
    return result

def Serial_Port_Configuration(port):
    global ser
    try:
        ser = serial.Serial(port, 115200,timeout=5)
        # ser = serial.Serial(port,9600,timeout=2)
    except:
        print("\n   Oops! That was not a valid port")
        
        port = serial_ports()
        if(not port):
            print("\n   No ports Detected")
        else:
            print("\n   Here are some available ports on your PC. Try Again!")
            print("\n   ",port)
        return -1
    if ser.is_open:
        print("\n   Port Open Success")
    else:
        print("\n   Port Open Failed")
    return 0

              
def read_serial_port(length):
    read_value = ser.read(length)
    return read_value

def Close_serial_port():
    pass
def purge_serial_port():
    ser.reset_input_buffer()
    
def Write_to_serial_port(value, *length):
        data = struct.pack('>B', value)
        if (verbose_mode):
            value = bytearray(data)
            #print("   "+hex(value[0]), end='')
            print("   "+"0x{:02x}".format(value[0]),end=' ')
        if(mem_write_active and (not verbose_mode)):
                print("#",end=' ')
        ser.write(data)


        
#----------------------------- command processing----------------------------------------

def process_COMMAND_BL_GO_TO_BOOTLOADER(length):
    boot_status=0
    value = read_serial_port(length)
    boot_status = bytearray(value)
    print("\n   Bootloader Status : ",hex(boot_status[0]))
    if(boot_status[0]==0x20):
        print("\n   Bootloader Active :")
    elif(boot_status[0]==0x15):
        print("\n   Bootloader Inactive :")
        if(boot_status[1]==0x17):
            print("\n   Jumping Bootloader :")
            print("\n   Bootloader Status : ",hex(boot_status[1]))

    else:
        print("\n   No Response :")





def process_COMMAND_BL_GET_VER(length):
    ver=read_serial_port(1)
    value = bytearray(ver)
    print("\n   Bootloader Ver. : ",hex(value[0]))

def process_COMMAND_BL_GET_HELP(length):
    #print("reading:", length)
    value = read_serial_port(length) 
    reply = bytearray(value)
    print("\n   Supported Commands :",end=' ')
    for x in reply:
        print(hex(x),end=' ')
    print()

def process_COMMAND_BL_GET_CID(length):
    value = read_serial_port(length)
    ci = (value[1] << 8 )+ value[0]
    print("\n   Chip Id. : ",hex(ci))

def process_COMMAND_BL_GET_RDP_STATUS(length):
    value = read_serial_port(length)
    rdp = bytearray(value)
    print("\n   RDP Status : ",hex(rdp[0]))

def process_COMMAND_BL_GO_TO_ADDR(length):
    addr_status=0
    value = read_serial_port(length)
    addr_status = bytearray(value)
    print("\n   Address Status : ",hex(addr_status[0]))

def process_COMMAND_BL_FLASH_ERASE(length):
    erase_status=0
    value = read_serial_port(length)
    if len(value):
        erase_status = bytearray(value)
        if(erase_status[0] == Flash_HAL_OK):
            print("\n   Erase Status: Success  Code: FLASH_HAL_OK")
        elif(erase_status[0] == Flash_HAL_ERROR):
            print("\n   Erase Status: Fail  Code: FLASH_HAL_ERROR")
        elif(erase_status[0] == Flash_HAL_BUSY):
            print("\n   Erase Status: Fail  Code: FLASH_HAL_BUSY")
        elif(erase_status[0] == Flash_HAL_TIMEOUT):
            print("\n   Erase Status: Fail  Code: FLASH_HAL_TIMEOUT")
        elif(erase_status[0] == Flash_HAL_INV_ADDR):
            print("\n   Erase Status: Fail  Code: FLASH_HAL_INV_SECTOR")
        else:
            print("\n   Erase Status: Fail  Code: UNKNOWN_ERROR_CODE")
    else:
        print("Timeout: Bootloader is not responding")

def process_COMMAND_BL_MEM_WRITE(length):
    write_status=0
    value = read_serial_port(length)
    write_status = bytearray(value)
    if(write_status[0] == Flash_HAL_OK):
        print("\n   Write_status: FLASH_HAL_OK")
    elif(write_status[0] == Flash_HAL_ERROR):
        print("\n   Write_status: FLASH_HAL_ERROR")
    elif(write_status[0] == Flash_HAL_BUSY):
        print("\n   Write_status: FLASH_HAL_BUSY")
    elif(write_status[0] == Flash_HAL_TIMEOUT):
        print("\n   Write_status: FLASH_HAL_TIMEOUT")
    elif(write_status[0] == Flash_HAL_INV_ADDR):
        print("\n   Write_status: FLASH_HAL_INV_ADDR")
    else:
        print("\n   Write_status: UNKNOWN_ERROR")
    print("\n")
    
def process_COMMAND_BL_EXT_MEM_WRITE(length):
    write_status=0
    value = read_serial_port(length)
    write_status = bytearray(value)
    if(write_status[0] == Flash_HAL_OK):
        print("\n   Write_status: FLASH_HAL_OK")
    elif(write_status[0] == Flash_HAL_ERROR):
        print("\n   Write_status: FLASH_HAL_ERROR")
    elif(write_status[0] == Flash_HAL_BUSY):
        print("\n   Write_status: FLASH_HAL_BUSY")
    elif(write_status[0] == Flash_HAL_TIMEOUT):
        print("\n   Write_status: FLASH_HAL_TIMEOUT")
    elif(write_status[0] == Flash_HAL_INV_ADDR):
        print("\n   Write_status: FLASH_HAL_INV_ADDR")
    else:
        print("\n   Write_status: UNKNOWN_ERROR")
    print("\n")


def process_COMMAND_BL_FLASH_MASS_ERASE(length):
    pass

def process_COMMAND_BL_EXT_MEM_TO_MEM_WRITE(length):
    boot_status=0
    value = read_serial_port(length)
    boot_status = bytearray(value)
    print("\n   Bootloader Status : ",hex(boot_status[0]))
    if(boot_status[0]==0x20):
        print("\n   Bootloader MEM_WRITE_WORKING :")
    else:
        print("\n   No Response :")


protection_mode= [ "Write Protection", "Read/Write Protection","No protection" ]
def protection_type(status,n):
    if( status & (1 << 15) ):
        #PCROP is active
        if(status & (1 << n) ):
            return protection_mode[1]
        else:
            return protection_mode[2]
    else:
        if(status & (1 << n)):
            return protection_mode[2]
        else:
            return protection_mode[0]
            
        
        
        
def process_COMMAND_BL_READ_SECTOR_STATUS(length):
    s_status=0

    value = read_serial_port(length)
    s_status = bytearray(value)
    #s_status.flash_sector_status = (uint16_t)(status[1] << 8 | status[0] )
    print("\n   Sector Status : ",s_status[0])
    print("\n  ====================================")
    print("\n  Sector                               \tProtection") 
    print("\n  ====================================")
    if(s_status[0] & (1 << 15)):
        #PCROP is active
        print("\n  Flash protection mode : Read/Write Protection(PCROP)\n")
    else:
        print("\n  Flash protection mode :   \tWrite Protection\n")

    for x in range(8):
        print("\n   Sector{0}                               {1}".format(x,protection_type(s_status[0],x) ) )
        


def process_COMMAND_BL_DIS_R_W_PROTECT(length):
    status=0
    value = read_serial_port(length)
    status = bytearray(value)
    if(status[0]):
        print("\n   FAIL")
    else:
        print("\n   SUCCESS")

def process_COMMAND_BL_EN_R_W_PROTECT(length):
    status=0
    value = read_serial_port(length)
    status = bytearray(value)
    if(status[0]):
        print("\n   FAIL")
    else:
        print("\n   SUCCESS")




def decode_menu_command_code(command):
    ret_value = 0
    data_buf = []
    for i in range(255):
        data_buf.append(0)
    
    if(command  == 0 ):
        print("\n   Exiting...!")
        raise SystemExit
    elif(command == 1):
        print("\n   Command == > BL_GET_VER")
        COMMAND_BL_GET_VER_LEN              = 6
        data_buf[0] = COMMAND_BL_GET_VER_LEN-1 
        data_buf[1] = COMMAND_BL_GET_VER 
        crc32       = get_crc(data_buf,COMMAND_BL_GET_VER_LEN-4)
        crc32 = crc32 & 0xffffffff
        data_buf[2] = word_to_byte(crc32,1,1) 
        data_buf[3] = word_to_byte(crc32,2,1) 
        data_buf[4] = word_to_byte(crc32,3,1) 
        data_buf[5] = word_to_byte(crc32,4,1) 

        Write_to_serial_port(00,1)
        Write_to_serial_port(15,1)
        Write_to_serial_port(17,1)
        Write_to_serial_port(data_buf[0],1)
        for i in data_buf[1:COMMAND_BL_GET_VER_LEN]:
            Write_to_serial_port(i,COMMAND_BL_GET_VER_LEN-1)
        

        ret_value = read_bootloader_reply(data_buf[1])
        
        

    elif(command == 2):
        print("\n   Command == > BL_GET_HELP")
        COMMAND_BL_GET_HELP_LEN             =6
        data_buf[0] = COMMAND_BL_GET_HELP_LEN-1 
        data_buf[1] = COMMAND_BL_GET_HELP 
        crc32       = get_crc(data_buf,COMMAND_BL_GET_HELP_LEN-4)
        crc32 = crc32 & 0xffffffff
        data_buf[2] = word_to_byte(crc32,1,1) 
        data_buf[3] = word_to_byte(crc32,2,1) 
        data_buf[4] = word_to_byte(crc32,3,1) 
        data_buf[5] = word_to_byte(crc32,4,1) 

        Write_to_serial_port(00,1)
        Write_to_serial_port(15,1)
        Write_to_serial_port(17,1)
        Write_to_serial_port(data_buf[0],1)
        for i in data_buf[1:COMMAND_BL_GET_HELP_LEN]:
            Write_to_serial_port(i,COMMAND_BL_GET_HELP_LEN-1)
        

        ret_value = read_bootloader_reply(data_buf[1])
    elif(command == 3):
        print("\n   Command == > BL_GET_CID")
        COMMAND_BL_GET_CID_LEN             =6
        data_buf[0] = COMMAND_BL_GET_CID_LEN-1 
        data_buf[1] = COMMAND_BL_GET_CID 
        crc32       = get_crc(data_buf,COMMAND_BL_GET_CID_LEN-4)
        crc32 = crc32 & 0xffffffff
        data_buf[2] = word_to_byte(crc32,1,1) 
        data_buf[3] = word_to_byte(crc32,2,1) 
        data_buf[4] = word_to_byte(crc32,3,1) 
        data_buf[5] = word_to_byte(crc32,4,1) 

        Write_to_serial_port(00,1)
        Write_to_serial_port(15,1)
        Write_to_serial_port(17,1)       
        Write_to_serial_port(data_buf[0],1)
        for i in data_buf[1:COMMAND_BL_GET_CID_LEN]:
            Write_to_serial_port(i,COMMAND_BL_GET_CID_LEN-1)
        

        ret_value = read_bootloader_reply(data_buf[1])

    elif(command == 4):
        print("\n   Command == > BL_GET_RDP_STATUS")
        data_buf[0] = COMMAND_BL_GET_RDP_STATUS_LEN-1
        data_buf[1] = COMMAND_BL_GET_RDP_STATUS
        crc32       = get_crc(data_buf,COMMAND_BL_GET_RDP_STATUS_LEN-4)
        crc32 = crc32 & 0xffffffff
        data_buf[2] = word_to_byte(crc32,1,1)
        data_buf[3] = word_to_byte(crc32,2,1)
        data_buf[4] = word_to_byte(crc32,3,1)
        data_buf[5] = word_to_byte(crc32,4,1)
        Write_to_serial_port(00,1)
        Write_to_serial_port(15,1)
        Write_to_serial_port(17,1)       
        Write_to_serial_port(data_buf[0],1)
        
        for i in data_buf[1:COMMAND_BL_GET_RDP_STATUS_LEN]:
            Write_to_serial_port(i,COMMAND_BL_GET_RDP_STATUS_LEN-1)
        
        ret_value = read_bootloader_reply(data_buf[1])
    elif(command == 5):
        print("\n   Command == > BL_GO_TO_ADDR")
        go_address  = input("\n   Please enter 4 bytes go address in hex:")
        go_address = int(go_address, 16)
        data_buf[0] = COMMAND_BL_GO_TO_ADDR_LEN-1 
        data_buf[1] = COMMAND_BL_GO_TO_ADDR 
        data_buf[2] = word_to_byte(go_address,1,1) 
        data_buf[3] = word_to_byte(go_address,2,1) 
        data_buf[4] = word_to_byte(go_address,3,1) 
        data_buf[5] = word_to_byte(go_address,4,1) 
        crc32       = get_crc(data_buf,COMMAND_BL_GO_TO_ADDR_LEN-4) 
        data_buf[6] = word_to_byte(crc32,1,1) 
        data_buf[7] = word_to_byte(crc32,2,1) 
        data_buf[8] = word_to_byte(crc32,3,1) 
        data_buf[9] = word_to_byte(crc32,4,1) 

        Write_to_serial_port(00,1)
        Write_to_serial_port(15,1)
        Write_to_serial_port(17,1)
        Write_to_serial_port(data_buf[0],1)
        
        for i in data_buf[1:COMMAND_BL_GO_TO_ADDR_LEN]:
            Write_to_serial_port(i,COMMAND_BL_GO_TO_ADDR_LEN-1)
        
        ret_value = read_bootloader_reply(data_buf[1])
        
    elif(command == 6):
        print("\n   This command is not supported")
    elif(command == 7):
        print("\n   Command == > BL_FLASH_ERASE")
        data_buf[0] = COMMAND_BL_FLASH_ERASE_LEN-1 
        data_buf[1] = COMMAND_BL_FLASH_ERASE 
        sector_num = input("\n   Enter sector number(0-11 or 0xFF) here :")
        sector_num = int(sector_num)
        if(sector_num != 0xff):
            nsec=int(input("\n   Enter number of sectors to erase(max 11) here :"))
        else:
            nsec = int(11)
        
        data_buf[2]= sector_num
        data_buf[3]= nsec 

        crc32       = get_crc(data_buf,COMMAND_BL_FLASH_ERASE_LEN-4) 
        data_buf[4] = word_to_byte(crc32,1,1) 
        data_buf[5] = word_to_byte(crc32,2,1) 
        data_buf[6] = word_to_byte(crc32,3,1) 
        data_buf[7] = word_to_byte(crc32,4,1) 


        Write_to_serial_port(00,1)
        Write_to_serial_port(15,1)
        Write_to_serial_port(17,1)
        Write_to_serial_port(data_buf[0],1)
        
        for i in data_buf[1:COMMAND_BL_FLASH_ERASE_LEN]:
            Write_to_serial_port(i,COMMAND_BL_FLASH_ERASE_LEN-1)
        
        ret_value = read_bootloader_reply(data_buf[1])
        
    elif(command == 8):
        print("\n   Command == > BL_MEM_WRITE")
        bytes_remaining=0
        t_len_of_file=0
        bytes_so_far_sent = 0
        len_to_read=0
        base_mem_address=0

        data_buf[1] = COMMAND_BL_MEM_WRITE

        #First get the total number of bytes in the .bin file.
        t_len_of_file =calc_file_len()

        #keep opening the file
        open_the_file()

        bytes_remaining = t_len_of_file - bytes_so_far_sent

        base_mem_address = input("\n   Enter the memory write address here :")
        base_mem_address = int(base_mem_address, 16)
        global mem_write_active
        while(bytes_remaining):
            mem_write_active=1
            if(bytes_remaining >= 128):
                len_to_read = 128
            else:
                len_to_read = bytes_remaining
            #get the bytes in to buffer by reading file
            for x in range(len_to_read):
                file_read_value = bin_file.read(1)
                file_read_value = bytearray(file_read_value)
                data_buf[7+x]= int(file_read_value[0])
                
            #read_the_file(&data_buf[7],len_to_read) 
            #print("\n   base mem address = \n",base_mem_address, hex(base_mem_address)) 
            



            #populate base mem address
            data_buf[2] = word_to_byte(base_mem_address,1,1)
            data_buf[3] = word_to_byte(base_mem_address,2,1)
            data_buf[4] = word_to_byte(base_mem_address,3,1)
            data_buf[5] = word_to_byte(base_mem_address,4,1)

            data_buf[6] = len_to_read

            #/* 1 byte len + 1 byte command code + 4 byte mem base address
            #* 1 byte payload len + len_to_read is amount of bytes read from file + 4 byte CRC
            #*/
            mem_write_cmd_total_len = COMMAND_BL_MEM_WRITE_LEN+len_to_read

            #first field is "len_to_follow"
            data_buf[0] =mem_write_cmd_total_len-1
            #deneme kodu
            uint32_values = []
            for x in range(0, (len_to_read+7), 4):
                uint32_value = (data_buf[x] << 24) | (data_buf[x + 1] << 16) | (data_buf[x + 2] << 8) | data_buf[x + 3]
                uint32_values.append(uint32_value)
            #deneme kodu
            crc32       = get_crc(data_buf,mem_write_cmd_total_len-4)
            #crc32       = get_crc(uint32_values,len(uint32_values))
            data_buf[7+len_to_read] = word_to_byte(crc32,1,1)
            data_buf[8+len_to_read] = word_to_byte(crc32,2,1)
            data_buf[9+len_to_read] = word_to_byte(crc32,3,1)
            data_buf[10+len_to_read] = word_to_byte(crc32,4,1)

            #update base mem address for the next loop
            base_mem_address+=len_to_read

            #oteleme_yapilan_dizi = [0x18, 0x66, 0x13] + data_buf[:-3]
            Write_to_serial_port(00,1)
            Write_to_serial_port(15,1)
            Write_to_serial_port(17,1)
            Write_to_serial_port(data_buf[0],1)

            for i in data_buf[1:mem_write_cmd_total_len]:

                Write_to_serial_port(i,mem_write_cmd_total_len-1)

            bytes_so_far_sent+=len_to_read
            bytes_remaining = t_len_of_file - bytes_so_far_sent
            print("\n   bytes_so_far_sent:{0} -- bytes_remaining:{1}\n".format(bytes_so_far_sent,bytes_remaining)) 
            #time.sleep(0.350)
        
            ret_value = read_bootloader_reply(data_buf[1])
            while not ((ret_value==0)):
                if(ret_value==-1):
                    print("\n   Packet has benn recent")
                    Write_to_serial_port(00,1)
                    Write_to_serial_port(15,1)
                    Write_to_serial_port(17,1)
                    Write_to_serial_port(data_buf[0],1)

                    for i in data_buf[1:mem_write_cmd_total_len]:

                        Write_to_serial_port(i,mem_write_cmd_total_len-1)

                    bytes_remaining = t_len_of_file - bytes_so_far_sent
                    print("\n   bytes_so_far_sent:{0} -- bytes_remaining:{1}\n".format(bytes_so_far_sent,bytes_remaining))
                    
                    ret_value = read_bootloader_reply(data_buf[1])

                elif(ret_value == 5):
                    print("\n   Packet has benn recent")
                    Write_to_serial_port(00,1)
                    Write_to_serial_port(15,1)
                    Write_to_serial_port(17,1)
                    Write_to_serial_port(data_buf[0],1)
                    for i in data_buf[1:mem_write_cmd_total_len]:
                        Write_to_serial_port(i,mem_write_cmd_total_len-1)

                    bytes_remaining = t_len_of_file - bytes_so_far_sent
                    print("\n   bytes_so_far_sent:{0} -- bytes_remaining:{1}\n".format(bytes_so_far_sent,bytes_remaining))

                    ret_value = read_bootloader_reply(data_buf[1])

        mem_write_active=0

            
    
    elif(command == 9):
        print("\n   Command == > BL_EN_R_W_PROTECT")
        total_sector = int(input("\n   How many sectors do you want to protect ?: "))
        sector_numbers = [0,0,0,0,0,0,0,0]
        sector_details=0
        for x in range(total_sector):
            sector_numbers[x]=int(input("\n   Enter sector number[{0}]: ".format(x+1) ))
            sector_details = sector_details | (1 << sector_numbers[x])

        #print("Sector details",sector_details)
        print("\n   Mode:Flash sectors Write Protection: 1")
        print("\n   Mode:Flash sectors Read/Write Protection: 2")
        mode=input("\n   Enter Sector Protection Mode(1 or 2 ):")
        mode = int(mode)
        if(mode != 2 and mode != 1):
            printf("\n   Invalid option : Command Dropped")
            return
        if(mode == 2):
            print("\n   This feature is currently not supported !") 
            return

        data_buf[0] = COMMAND_BL_EN_R_W_PROTECT_LEN-1 
        data_buf[1] = COMMAND_BL_EN_R_W_PROTECT 
        data_buf[2] = sector_details 
        data_buf[3] = mode 
        crc32       = get_crc(data_buf,COMMAND_BL_EN_R_W_PROTECT_LEN-4) 
        data_buf[4] = word_to_byte(crc32,1,1) 
        data_buf[5] = word_to_byte(crc32,2,1) 
        data_buf[6] = word_to_byte(crc32,3,1) 
        data_buf[7] = word_to_byte(crc32,4,1) 

        Write_to_serial_port(00,1)
        Write_to_serial_port(15,1)
        Write_to_serial_port(17,1)
        Write_to_serial_port(data_buf[0],1)
        
        for i in data_buf[1:COMMAND_BL_EN_R_W_PROTECT_LEN]:
            Write_to_serial_port(i,COMMAND_BL_EN_R_W_PROTECT_LEN-1)
        
        ret_value = read_bootloader_reply(data_buf[1])
            
        
    elif(command == 10):
        print("\n   Command == > COMMAND_BL_MEM_READ")
        print("\n   This command is not supported")
    elif(command == 11):
        print("\n   Command == > COMMAND_BL_READ_SECTOR_P_STATUS")
        data_buf[0] = COMMAND_BL_READ_SECTOR_P_STATUS_LEN-1 
        data_buf[1] = COMMAND_BL_READ_SECTOR_P_STATUS 

        crc32       = get_crc(data_buf,COMMAND_BL_READ_SECTOR_P_STATUS_LEN-4) 
        data_buf[2] = word_to_byte(crc32,1,1) 
        data_buf[3] = word_to_byte(crc32,2,1) 
        data_buf[4] = word_to_byte(crc32,3,1) 
        data_buf[5] = word_to_byte(crc32,4,1) 

        Write_to_serial_port(data_buf[0],1)
        
        for i in data_buf[1:COMMAND_BL_READ_SECTOR_P_STATUS_LEN]:
            Write_to_serial_port(i,COMMAND_BL_READ_SECTOR_P_STATUS_LEN-1)
        
        ret_value = read_bootloader_reply(data_buf[1])

    elif(command == 12):
        print("\n   Command == > COMMAND_OTP_READ")
        print("\n   This command is not supported")
    elif(command == 13):
        print("\n   Command == > COMMAND_BL_DIS_R_W_PROTECT")
        data_buf[0] = COMMAND_BL_DIS_R_W_PROTECT_LEN-1 
        data_buf[1] = COMMAND_BL_DIS_R_W_PROTECT 
        crc32       = get_crc(data_buf,COMMAND_BL_DIS_R_W_PROTECT_LEN-4) 
        data_buf[2] = word_to_byte(crc32,1,1) 
        data_buf[3] = word_to_byte(crc32,2,1) 
        data_buf[4] = word_to_byte(crc32,3,1) 
        data_buf[5] = word_to_byte(crc32,4,1) 

        Write_to_serial_port(data_buf[0],1)
        
        for i in data_buf[1:COMMAND_BL_DIS_R_W_PROTECT_LEN]:
            Write_to_serial_port(i,COMMAND_BL_DIS_R_W_PROTECT_LEN-1)
        
        ret_value = read_bootloader_reply(data_buf[1])
        
    elif(command == 14):
        print("\n   Command == > GO_TO_BOOTLADER_APP")
        go_address = Flash_BOOTLOADER_ADDR
        #go_address = int(go_address, 16)
        data_buf[0] = COMMAND_BL_GO_TO_ADDR_LEN-1 
        data_buf[1] = COMMAND_BL_GO_TO_BOOTLOADER 
        data_buf[2] = word_to_byte(go_address,1,1) 
        data_buf[3] = word_to_byte(go_address,2,1) 
        data_buf[4] = word_to_byte(go_address,3,1) 
        data_buf[5] = word_to_byte(go_address,4,1) 
        crc32       = get_crc(data_buf,COMMAND_BL_GO_TO_ADDR_LEN-4) 
        data_buf[6] = word_to_byte(crc32,1,1) 
        data_buf[7] = word_to_byte(crc32,2,1) 
        data_buf[8] = word_to_byte(crc32,3,1) 
        data_buf[9] = word_to_byte(crc32,4,1) 

        Write_to_serial_port(00,1)
        Write_to_serial_port(15,1)
        Write_to_serial_port(17,1)
        Write_to_serial_port(data_buf[0],1)
        
        for i in data_buf[1:COMMAND_BL_GO_TO_ADDR_LEN]:
            Write_to_serial_port(i,COMMAND_BL_GO_TO_ADDR_LEN-1)
        
        ret_value = read_bootloader_reply(data_buf[1])
    elif(command == 15):
        print("\n   Command == > BL_EXT_MEM_WRITE")
        bytes_remaining=0
        t_len_of_file=0
        bytes_so_far_sent = 0
        len_to_read=0
        base_mem_address=0

        data_buf[1] = COMMAND_BL_EXT_MEM_WRITE
        #First get the total number of bytes in the .bin file.
        t_len_of_file =calc_file_len()

        #keep opening the file
        open_the_file()

        bytes_remaining = t_len_of_file - bytes_so_far_sent

        base_mem_address = input("\n   Enter the memory write address here :")
        base_mem_address = int(base_mem_address, 16)
        global mem_ext_write_active
        while(bytes_remaining):
            mem_ext_write_active=1
            if(bytes_remaining >= 128):
                len_to_read = 128
            else:
                len_to_read = bytes_remaining
            #get the bytes in to buffer by reading file
            for x in range(len_to_read):
                file_read_value = bin_file.read(1)
                file_read_value = bytearray(file_read_value)
                data_buf[7+x]= int(file_read_value[0])
                
            #read_the_file(&data_buf[7],len_to_read) 
            #print("\n   base mem address = \n",base_mem_address, hex(base_mem_address)) 
            



            #populate base mem address
            data_buf[2] = word_to_byte(base_mem_address,1,1)
            data_buf[3] = word_to_byte(base_mem_address,2,1)
            data_buf[4] = word_to_byte(base_mem_address,3,1)
            data_buf[5] = word_to_byte(base_mem_address,4,1)

            data_buf[6] = len_to_read

            #/* 1 byte len + 1 byte command code + 4 byte mem base address
            #* 1 byte payload len + len_to_read is amount of bytes read from file + 4 byte CRC
            #*/
            mem_write_cmd_total_len = COMMAND_BL_MEM_WRITE_LEN+len_to_read

            #first field is "len_to_follow"
            data_buf[0] =mem_write_cmd_total_len-1
            #deneme kodu
            uint32_values = []
            for x in range(0, (len_to_read+7), 4):
                uint32_value = (data_buf[x] << 24) | (data_buf[x + 1] << 16) | (data_buf[x + 2] << 8) | data_buf[x + 3]
                uint32_values.append(uint32_value)
            #deneme kodu
            crc32       = get_crc(data_buf,mem_write_cmd_total_len-4)
            #crc32       = get_crc(uint32_values,len(uint32_values))
            data_buf[7+len_to_read] = word_to_byte(crc32,1,1)
            data_buf[8+len_to_read] = word_to_byte(crc32,2,1)
            data_buf[9+len_to_read] = word_to_byte(crc32,3,1)
            data_buf[10+len_to_read] = word_to_byte(crc32,4,1)

            #update base mem address for the next loop
            base_mem_address+=len_to_read

            #oteleme_yapilan_dizi = [0x18, 0x66, 0x13] + data_buf[:-3]
            Write_to_serial_port(00,1)
            Write_to_serial_port(15,1)
            Write_to_serial_port(17,1)
            Write_to_serial_port(data_buf[0],1)

            for i in data_buf[1:mem_write_cmd_total_len]:

                Write_to_serial_port(i,mem_write_cmd_total_len-1)

            bytes_so_far_sent+=len_to_read
            bytes_remaining = t_len_of_file - bytes_so_far_sent
            print("\n   bytes_so_far_sent:{0} -- bytes_remaining:{1}\n".format(bytes_so_far_sent,bytes_remaining)) 
            #time.sleep(0.350)
        
            ret_value = read_bootloader_reply(data_buf[1])
            while not ((ret_value==0)):
                if(ret_value==-1): #CRC FAIL ise yine gönder
                    print("\n   Packet has benn recent")
                    Write_to_serial_port(00,1)
                    Write_to_serial_port(15,1)
                    Write_to_serial_port(17,1)
                    Write_to_serial_port(data_buf[0],1)

                    for i in data_buf[1:mem_write_cmd_total_len]:

                        Write_to_serial_port(i,mem_write_cmd_total_len-1)

                    bytes_remaining = t_len_of_file - bytes_so_far_sent
                    print("\n   bytes_so_far_sent:{0} -- bytes_remaining:{1}\n".format(bytes_so_far_sent,bytes_remaining))
                    
                    ret_value = read_bootloader_reply(data_buf[1])

                elif(ret_value == 5):   # booltoader cevap gelmesse tekrar gönder
                    print("\n   Packet has benn recent")
                    Write_to_serial_port(00,1)
                    Write_to_serial_port(15,1)
                    Write_to_serial_port(17,1)
                    Write_to_serial_port(data_buf[0],1)
                    for i in data_buf[1:mem_write_cmd_total_len]:
                        Write_to_serial_port(i,mem_write_cmd_total_len-1)

                    bytes_remaining = t_len_of_file - bytes_so_far_sent
                    print("\n   bytes_so_far_sent:{0} -- bytes_remaining:{1}\n".format(bytes_so_far_sent,bytes_remaining))

                    ret_value = read_bootloader_reply(data_buf[1])
                    
                elif(ret_value == 3):   # Bootloader Sonraki paketi gönder derse
                    print("\n   Sending Next Packet")
                    Write_to_serial_port(00,1)
                    Write_to_serial_port(15,1)
                    Write_to_serial_port(17,1)
                    Write_to_serial_port(data_buf[0],1)
                    for i in data_buf[1:mem_write_cmd_total_len]:
                        Write_to_serial_port(i,mem_write_cmd_total_len-1)

                    bytes_remaining = t_len_of_file - bytes_so_far_sent
                    print("\n   bytes_so_far_sent:{0} -- bytes_remaining:{1}\n".format(bytes_so_far_sent,bytes_remaining))

                    ret_value = read_bootloader_reply(data_buf[1])     

        mem_ext_write_active=0

    elif(command == 16):
            print("\n   Command == > BL_EXT_MEM_TO_MEM_WRITE")
            COMMAND_BL_EXT_MEM_TO_MEM_WRITE_LEN              = 6
            data_buf[0] = COMMAND_BL_EXT_MEM_TO_MEM_WRITE_LEN-1 
            data_buf[1] = COMMAND_BL_EXT_MEM_TO_MEM_WRITE
            crc32       = get_crc(data_buf,COMMAND_BL_EXT_MEM_TO_MEM_WRITE_LEN-4)
            crc32 = crc32 & 0xffffffff
            data_buf[2] = word_to_byte(crc32,1,1) 
            data_buf[3] = word_to_byte(crc32,2,1) 
            data_buf[4] = word_to_byte(crc32,3,1) 
            data_buf[5] = word_to_byte(crc32,4,1) 

            Write_to_serial_port(00,1)
            Write_to_serial_port(15,1)
            Write_to_serial_port(17,1)
            Write_to_serial_port(data_buf[0],1)
            for i in data_buf[1:COMMAND_BL_EXT_MEM_TO_MEM_WRITE_LEN]:
                Write_to_serial_port(i,COMMAND_BL_EXT_MEM_TO_MEM_WRITE_LEN-1)
            

            ret_value = read_bootloader_reply(data_buf[1])
def read_bootloader_reply(command_code):
    #ack=[0,0]
    len_to_follow=0 
    ret = -2 

    #read_serial_port(ack,2)
    #ack = ser.read(2)
    ack=read_serial_port(2)
    if(len(ack) ):
        a_array=bytearray(ack)
        #print("read uart:",ack) 
        if (a_array[0]== 0xA5):
            #CRC of last command was good .. received ACK and "len to follow"
            len_to_follow=a_array[1]
            #len_to_follow=1 deneysel birsey kodla alakasi yok
            print("\n   CRC : SUCCESS Len :",len_to_follow)
            #print("command_code:",hex(command_code))
            if 1==1:
        
                if (command_code) == COMMAND_BL_GET_VER :
                    process_COMMAND_BL_GET_VER(len_to_follow)
                    
                elif(command_code) == COMMAND_BL_GET_HELP:
                    process_COMMAND_BL_GET_HELP(len_to_follow)
                    
                elif(command_code) == COMMAND_BL_GET_CID:
                    process_COMMAND_BL_GET_CID(len_to_follow)
                    
                elif(command_code) == COMMAND_BL_GET_RDP_STATUS:
                    process_COMMAND_BL_GET_RDP_STATUS(len_to_follow)
                    
                elif(command_code) == COMMAND_BL_GO_TO_ADDR:
                    process_COMMAND_BL_GO_TO_ADDR(len_to_follow)
                    
                elif(command_code) == COMMAND_BL_FLASH_ERASE:
                    process_COMMAND_BL_FLASH_ERASE(len_to_follow)
                    
                elif(command_code) == COMMAND_BL_MEM_WRITE:
                    process_COMMAND_BL_MEM_WRITE(len_to_follow)
                    
                elif(command_code) == COMMAND_BL_READ_SECTOR_P_STATUS:
                    process_COMMAND_BL_READ_SECTOR_STATUS(len_to_follow)
                    
                elif(command_code) == COMMAND_BL_EN_R_W_PROTECT:
                    process_COMMAND_BL_EN_R_W_PROTECT(len_to_follow)
                    
                elif(command_code) == COMMAND_BL_DIS_R_W_PROTECT:
                    process_COMMAND_BL_DIS_R_W_PROTECT(len_to_follow)
                    
                elif(command_code) == COMMAND_BL_GO_TO_BOOTLOADER:
                    process_COMMAND_BL_GO_TO_BOOTLOADER(len_to_follow)

                elif(command_code) == COMMAND_BL_EXT_MEM_WRITE:
                    process_COMMAND_BL_EXT_MEM_WRITE(len_to_follow)

                elif(command_code) == COMMAND_BL_EXT_MEM_TO_MEM_WRITE:
                    process_COMMAND_BL_EXT_MEM_TO_MEM_WRITE(len_to_follow)
                    
                else:
                    print("\n   Invalid command code\n")
                    
            ret = 0
         
        elif a_array[0] == 0x7F:
            #CRC of last command was bad .. received NACK
            print("\n   CRC: FAIL \n")
            ret= -1
    else:
        print("\n   Timeout : Bootloader not responding")
        ret = 5

    return ret

            
            

#----------------------------- Ask Menu implementation----------------------------------------


name = input("Enter the Port Name of your device(Ex: COM1):")
ret = 0
ret=Serial_Port_Configuration(name)
if(ret < 0):
    decode_menu_command_code(0)
    

    
  
while True:
    print("\n +==========================================+")
    print(" |               Menu                       |")
    print(" |         STM32F4 BootLoader               |")
    print(" +==========================================+")

  
    
    print("\n   Which BL command do you want to send ??\n")
    print("\n   Before writing new code to Flash, all users code must be deleted !!\n")
    print("   BL_GET_VER                            --> 1")
    print("   BL_GET_HLP                            --> 2")
    print("   BL_GET_CID                            --> 3")
    print("   BL_GET_RDP_STATUS                     --> 4")
    print("   BL_GO_TO_ADDR                         --> 5")
    print("   BL_FLASH_MASS_ERASE                   --> 6")
    print("   BL_FLASH_ERASE                        --> 7")
    print("   BL_MEM_WRITE                          --> 8")
    print("   BL_EN_R_W_PROTECT                     --> 9")
    print("   BL_MEM_READ                           --> 10")
    print("   BL_READ_SECTOR_P_STATUS               --> 11")
    print("   BL_OTP_READ                           --> 12")
    print("   BL_DIS_R_W_PROTECT                    --> 13")
    print("   GO_TO_BOOTLADER_APP                   --> 14")
    print("   BL_EXT_MEM_WRITE                      --> 15")
    print("   BL_EXT_MEM_TO_MEM_WRITE               --> 16")


    print("   MENU_EXIT                             --> 0")

    #command_code = int(input("\n   Type the command code here :") )

    command_code = input("\n   Type the command code here :")

    if(not command_code.isdigit()):
        print("\n   Please Input valid code shown above")
    else:
        decode_menu_command_code(int(command_code))

    input("\n   Press any key to continue  :")
    purge_serial_port()





    

def check_flash_status():
    pass

def protection_type():
    pass

