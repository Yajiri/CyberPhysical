xhost +

docker run --rm -ti --init --ipc=host -v /tmp:/tmp -e DISPLAY=$DISPLAY hsv:latest --name=img --width=640 --height=480