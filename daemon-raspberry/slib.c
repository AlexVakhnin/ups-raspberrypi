#include <fcntl.h>  /* File Control Definitions          */
#include <termios.h>/* POSIX Terminal Control Definitions*/
#include <unistd.h> /* UNIX Standard Definitions         */
//#include <errno.h>  /* ERROR Number Definitions          */
#include <stdio.h>
//#include <plog.h>

int serial_init(int fd)
{
    struct termios SerialPortSettings;  //обьявляем структуру для настройки Serial

    if (tcgetattr(fd, &SerialPortSettings) < 0) {  //загружаем в структуру текущие значения настроек Serial
       return -1;
    }
    //заполнение структуры
    cfsetispeed(&SerialPortSettings,B9600);  //скорость на прием
    cfsetospeed(&SerialPortSettings,B9600);  //скорость на передачу
    SerialPortSettings.c_cflag &= ~PARENB; //не использовать бит паритета
    SerialPortSettings.c_cflag &= ~CSTOPB; //Stop bits = 1
    SerialPortSettings.c_cflag &= ~CSIZE; /* Clears the Mask       */
    SerialPortSettings.c_cflag |=  CS8;   /* Set the data bits = 8 */
    SerialPortSettings.c_cflag &= ~CRTSCTS; //выключаем flow control (RTS/CTS)
    SerialPortSettings.c_cflag |= CREAD | CLOCAL; //включаем приемник Serial
    SerialPortSettings.c_cc[VMIN]  = 0; //миним. колич. принятых символов перед выходом из read() 
    SerialPortSettings.c_cc[VTIME] = 5;  //время ожидания символов, 0-бесконечно
    SerialPortSettings.c_iflag &= ~(IXON | IXOFF | IXANY); //выключаем flow control (XON/XOFF)
    SerialPortSettings.c_iflag &= ~(ICANON | ECHO | ECHOE | ISIG); //NON Cannonical mode is recommended.

    if (tcsetattr(fd,TCSANOW,&SerialPortSettings) != 0) {   //выгружаем структуру в Serial
        return -1;
    }
    int n;
    n=write(fd,"\n",1);
    if (n != 1) {  //инициализируем сам модем
        return -1;
    }
    sleep(2);  //дать время модему отработать!!!
    //printf("init_write_count: %d\n",n);

    return 0;
}
