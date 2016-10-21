#!/bin/sh

clear
echo " ================================================================= "
echo "|  Maligno - A Metasploit Payload Server - Install Script         |"
echo "|  by Juan J. Guelfo, Encripto AS (post@encripto.no)              |"
echo " ================================================================= "

echo "\n\n\033[1;34m[*]\033[1;m Installing dependencies (python-ipcalc)...\n"
sleep 2
sudo apt-get update && sudo apt-get install -y python-ipcalc

echo "\n\n\033[1;34m[*]\033[1;m Creating certs folder..."
sleep 2
if [ ! -d "certs" ]; then
    mkdir certs
    echo "\033[1;32m[+]\033[1;m Directory successfully created."

else
    echo "\033[1;32m[+]\033[1;m Directory already exists."
fi

cd certs

echo "\n\n\033[1;34m[*]\033[1;m Generating server key and certificate...\n"
sleep 2
openssl req -nodes -new -x509 -days 3650 -keyout server.key -out server.crt

echo "\n\n\033[1;34m[*]\033[1;m Generating PEM..."
sleep 2
cat server.crt server.key > server.pem
echo "\033[1;32m[+]\033[1;m Certificate successfully generated."

echo "\n\n\033[1;32m[+]\033[1;m Installation completed! You can use server.pem with Maligno."
echo "    Don't forget to update your server config!\n"