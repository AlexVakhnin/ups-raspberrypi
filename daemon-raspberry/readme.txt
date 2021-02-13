1. File upsd (executable) put to /var/pi_control/cdaemon/
2. File upsd.service put to /etc/systemd/system/
3. systemctl enable upsd.service   -  for registration
4. systemctl start upsd   -   run as service
5. systemctl -l status upsd    -    check status