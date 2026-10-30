#aws coturn settings

sudo apt install coturn -y




sudo nano /etc/default/coturn

TURNSERVER_ENABLED=1



sudo nano /etc/turnserver.conf





listening-port=3478
listening-ip=172.31.32.136
external-ip=3.81.104.191/172.31.32.136
fingerprint
lt-cred-mech
user=testuser:testpassword
realm=3.81.104.191
# Force long-term credential mechanism
lt-cred-mech

# Enable logging to standard output/syslog for easy tracking
verbose
# Enable full packet dump in the log file
log-binding







sudo systemctl restart coturn
sudo systemctl enable coturn



https://webrtc.github.io/samples/src/content/peerconnection/trickle-ice/