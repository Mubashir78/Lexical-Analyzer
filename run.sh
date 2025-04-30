#!/bin/bash

# Compile only if needed
if [ ! -f ./math ] || [ lexical_analyzer.cpp -nt ./math ]; then
    g++ -std=c++11 lexical_analyzer.cpp -o math || exit 1
fi

# Start backend
python3 server.py &
sleep 2  # Wait for initialization

# Open browser
xdg-open "http://localhost:8000/index.html" 2>/dev/null

# Cleanup on exit
trap "kill $! 2>/dev/null" EXIT
wait