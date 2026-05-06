#!/bin/bash
# Upload firmware to the ESP32.
# Usage: ./upload.sh /dev/tty.usbserial-XXXX
#        ./upload.sh /dev/tty.usbserial-XXXX config.py   (single file)

set -e

PORT="${1:?Usage: $0 <port> [filename]}"
FILE="$2"

if [ -n "$FILE" ]; then
    echo "Uploading $FILE..."
    mpremote connect "$PORT" cp "firmware/$FILE" ":$FILE"
else
    echo "Uploading all firmware files to $PORT..."
    for f in firmware/*.py; do
        name="$(basename "$f")"
        echo "  $name"
        mpremote connect "$PORT" cp "$f" ":$name"
    done
fi

echo "Done. Rebooting board..."
mpremote connect "$PORT" exec "import machine; machine.reset()"
