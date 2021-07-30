#!/bin/bash
# This shell script gets a UUID and converts it to a C-style array of bytes
# representation
UUID=$(uuidgen -t)
echo "0x${UUID:34:2},0x${UUID:32:2},0x${UUID:30:2},0x${UUID:28:2},0x${UUID:26:2},0x${UUID:24:2},0x${UUID:21:2},0x${UUID:19:2},0x${UUID:16:2},0x${UUID:14:2},0x${UUID:11:2},0x${UUID:9:2},0x${UUID:6:2},0x${UUID:4:2},0x${UUID:2:2},0x${UUID:0:2}"
