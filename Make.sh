#!/bin/bash
g++ App.cpp Blake512.cpp Config.cpp Curl.cpp json_reader.cpp json_value.cpp json_writer.cpp main.cpp AppOpenCL.cpp RSHash.cpp SHA256.cpp Util.cpp -lpthread -lcurl -lOpenCL -o reaper
