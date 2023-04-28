# Read the ARGB pixels in Shared Memory (/tmp/img) and displays them on a 640x480 GUI
xhost +
docker run --rm -ti --net=host --ipc=host -e DISPLAY=$DISPLAY -v /tmp:/tmp app --cid=253 --name=img --width=640 --height=480 --verbose 