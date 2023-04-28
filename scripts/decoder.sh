# Consumer and Provider microservice
# Waits for a h264 encoded stream, decodes it and writes the ARGB pixels to Shared Memory (/tmp/img)
# The h264 encoded stream is provided by OpenDLV View playback

docker run --rm -ti --net=host --ipc=host -e DISPLAY=$DISPLAY -v /tmp:/tmp h264decoder:v0.0.5 --cid=253 --name=img