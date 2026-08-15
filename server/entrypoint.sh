#!/bin/sh
set -e

echo "Running seed..."
./build/server -s

echo "Starting main server..."
./build/server
