//version 1.2 stable

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <conio.h>

#include <fcntl.h>  /* File Control Definitions          */
#include <termios.h>/* POSIX Terminal Control Definitions*/
#include <unistd.h> /* UNIX Standard Definitions         */
//#include <errno.h>  /* ERROR Number Definitions          */

#include "plog.h"
#include "slib.h"



  char write_ati[] = "ati\n"; //�������� ������� � �������
  char write_atb[] = "atb\n"; //��������� ������� 1-����� 0-������ ������� ������
  char write_at2[] = "at2\n"; //������� 220v 1-���� �������
  char write_atz[] = "atz\n"; //������������� ��������� ������� shutdown
  int  bytes_written  =  0 ;
  char read_buffer[128];
  char read_buffer1[128];
  char read_buffer_ati[128];
  int i;
  int  bytes_read=0;
  int  bytes_read_ati=0;
  int count = 0;
  int fd;


void Plog_ati()
{
  bytes_written = write(fd,write_ati,sizeof(write_ati)-1); //�������� ������� "ati"
  sleep(2);
  for (i=0;i<128;i++){read_buffer1[i]=0;} //�������� ����� ������
  bytes_read_ati = read(fd,&read_buffer1,126); //������ ����� �����������
  for (i=0;i<128;i++){read_buffer_ati[i]=0;} //�������� ����� ������
  strncpy(read_buffer_ati,read_buffer1,36); //�������� ��������� � ����������
  Plog(read_buffer_ati);
}


int main(int argc, char **argv)
{

  char charger_state = '1';  //��������� �������� - ������� ����...

  sleep(5); //��������

  fd = open("/dev/ttyUSB0",O_RDWR | O_NOCTTY); //��������� ���������� Serial
  if(fd==1) {
     Plog("Error! in Opening ttyUSB0");
     //system("tail -6 upsd_log.txt");
     return -1;
     }
  else
     Plog("ttyUSB0 Opened Successfully");

   sleep(5);  //��������

  if (serial_init(fd)==0)    //������������� Serial
	Plog("Init ttyUSB0 Successful");
  else
        {
	Plog("Error! Init ttyUSB0");
        //system("tail -6 upsd_log.txt");
        return -1;
        }

for (;;)
{
//-----------------------------------------------------------------------------
  bytes_written = write(fd,write_atb,sizeof(write_atb)-1); //�������� ������� "atb"
  sleep(2);
  for (i=0;i<128;i++){read_buffer[i]=0;} //�������� ����� ������
  bytes_read = read(fd,&read_buffer,126); //������ ����� �����������
	if (read_buffer[0]=='0'){
	  Plog_ati();
	  bytes_written = write(fd,write_atz,sizeof(write_atz)-1); //�������� ������� "atz"
  	  sleep(2);
  	  for (i=0;i<128;i++){read_buffer[i]=0;} //�������� ����� ������
  	  bytes_read = read(fd,&read_buffer,126); //������ ����� �����������
	  break; //��� ������� sutdown ���������....
	}

//-----
  bytes_written = write(fd,write_at2,sizeof(write_at2)-1); //�������� ������� "at2"
  sleep(2);
  for (i=0;i<128;i++){read_buffer[i]=0;} //�������� ����� ������
  bytes_read = read(fd,&read_buffer,126); //������ ����� �����������

  if (read_buffer[0]!=charger_state){
	if (read_buffer[0]=='0'){Plog("220v -> off");Plog_ati();} else {Plog("220v -> on");Plog_ati();}
  	charger_state=read_buffer[0]; //�������� ���������� ��������
  }

  count++;

sleep(2); //�������� 1 �.
//-----------------------------------------------------------------------------
} //����������� ����

  close(fd);

  Plog("Shutdown process running...");

  system("init 0");

  return 0;
}
