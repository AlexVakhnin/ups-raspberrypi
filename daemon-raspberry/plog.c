#include <time.h>
#include <stdio.h>
#include "plog.h"

void Plog(char* s)
{
    FILE* f;

    f = fopen("/var/pi_control/cdaemon/upsd_log.txt", "a+");
    if (f)
    {
	pdatetime (f);
        fprintf(f, "%s\n", s);
        fclose(f);
    }
}

void  pdatetime(FILE* f)
{
    time_t T= time(NULL);
    struct  tm tm = *localtime(&T);

   fprintf(f,"[%02d/%02d/%04d ",tm.tm_mday, tm.tm_mon+1, tm.tm_year+1900);
   fprintf(f,"%02d:%02d:%02d] ",tm.tm_hour, tm.tm_min, tm.tm_sec);

//    return 0;
}

