#!/bin/bash
stty -F /dev/ttyACM0 -hupcl -echo -crtscts -icanon -opost -isig 9600
echo "Content-type: text/plain"
echo ""

if [ -n "$QUERY_STRING" ]; then
    query=$QUERY_STRING
    mode=$(echo "$query" | grep -oP '(?<=mode=)[^&]*')
    param=$(echo "$query" | grep -oP '(?<=param=)[^&]*')
else
    mode=$1
    param=$2
fi

if [ "$mode" == "camera" ]; then
    sudo /var/www/html/main camera True
    echo "Camera Captured!"
elif [ "$mode" == "segment" ]; then
    sudo /var/www/html/main segment "$param"
    echo "Segment Sent: $param"
elif [ "$mode" == "lcd" ]; then
    sudo /var/www/html/main lcd "$param"
    echo "LCD Updated: $param"
else
    echo "Usage: sender.sh [camera|segment|lcd] <value>"
fi
