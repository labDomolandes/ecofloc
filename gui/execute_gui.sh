#!/bin/bash

source /opt/ecofloc/gui/venv/bin/activate
python /opt/ecofloc/gui/flocUI.py &
FLOC_PID=$!

# KILL
cleanup() {
    echo "Terminating flocUI.py process..."
    kill $FLOC_PID
    wait $FLOC_PID
    deactivate
    exit
}

# Trap SIGINT (Ctrl+C) and other termination signals to run cleanup
trap cleanup SIGINT SIGTERM

# I need to wait the python script before launching the browser
sleep 2 #enough??

# Detect the current browser and launch the URL
URL="http://127.0.0.1:8050/"
echo "Please, open your web browser as a non-root user and tap ->  $URL"

# if command -v xdg-open &> /dev/null; then
#     xdg-open "$URL"
# elif command -v gnome-open &> /dev/null; then
#     gnome-open "$URL"
# elif command -v open &> /dev/null; then
#     open "$URL"
# else
#     echo "Please, open your web browser and tap ->  $URL"
# fi

# Wait for the GUI process to complete
wait $FLOC_PID

# Run cleanup if the process exits naturally
cleanup
