#include <fcntl.h>  /* File Control Definitions          */
#include <termios.h>/* POSIX Terminal Control Definitions*/
#include <unistd.h> /* UNIX Standard Definitions         */
//#include <errno.h>  /* ERROR Number Definitions          */
#include <stdio.h>
//#include <plog.h>

int serial_init(int fd)
{
    struct termios SerialPortSettings;  //��������� ��������� ��� ��������� Serial

    if (tcgetattr(fd, &SerialPortSettings) < 0) {  //��������� � ��������� ������� �������� �������� Serial
       return -1;
    }
    //���������� ���������
    cfsetispeed(&SerialPortSettings,B9600);  //�������� �� �����
    cfsetospeed(&SerialPortSettings,B9600);  //�������� �� ��������
    SerialPortSettings.c_cflag &= ~PARENB; //�� ������������ ��� ��������
    SerialPortSettings.c_cflag &= ~CSTOPB; //Stop bits = 1
    SerialPortSettings.c_cflag &= ~CSIZE; /* Clears the Mask       */
    SerialPortSettings.c_cflag |=  CS8;   /* Set the data bits = 8 */
    SerialPortSettings.c_cflag &= ~CRTSCTS; //��������� flow control (RTS/CTS)
    SerialPortSettings.c_cflag |= CREAD | CLOCAL; //�������� �������� Serial
    SerialPortSettings.c_cc[VMIN]  = 0; //�����. �����. �������� �������� ����� ������� �� read() 
    SerialPortSettings.c_cc[VTIME] = 5;  //����� �������� ��������, 0-����������
    SerialPortSettings.c_iflag &= ~(IXON | IXOFF | IXANY); //��������� flow control (XON/XOFF)
    SerialPortSettings.c_iflag &= ~(ICANON | ECHO | ECHOE | ISIG); //NON Cannonical mode is recommended.

    if (tcsetattr(fd,TCSANOW,&SerialPortSettings) != 0) {   //��������� ��������� � Serial
        return -1;
    }
    int n;
    n=write(fd,"\n",1);
    if (n != 1) {  //�������������� ��� �����
        return -1;
    }
    sleep(2);  //���� ����� ������ ����������!!!
    //printf("init_write_count: %d\n",n);

    return 0;
}
