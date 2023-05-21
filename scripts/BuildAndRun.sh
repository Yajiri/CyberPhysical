#!/bin/bash
cd ..
docker build -t app -f Dockerfile .
xhost +
docker run --rm -ti --net=host --ipc=host -e DISPLAY=$DISPLAY -v /tmp:/tmp app --cid=253 --name=img --width=640 --height=480 --verbose
