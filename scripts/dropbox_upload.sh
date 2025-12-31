#!/bin/bash

# Usage: ./dropbox_upload.sh <local_file> <dropbox_path>
# Example dropbox_path: "/folder/file.txt"
# Requires: DROPBOX_TOKEN env variable set to your Dropbox API token

if [ $# -ne 2 ]; then
  echo "Usage: $0 <local_file> <dropbox_path>"
  exit 1
fi

LOCAL_FILE="$1"
DROPBOX_PATH="$2"

if [ -z "$DROPBOX_TOKEN" ]; then
  echo "Please set DROPBOX_TOKEN environment variable with your Dropbox API token."
  exit 1
fi

FILE_SIZE=$(stat -c%s "$LOCAL_FILE")

curl -X POST https://content.dropboxapi.com/2/files/upload \
  --header "Authorization: Bearer $DROPBOX_TOKEN" \
  --header "Dropbox-API-Arg: {\"path\": \"$DROPBOX_PATH\",\"mode\": \"overwrite\",\"autorename\": true,\"mute\": false}" \
  --header "Content-Type: application/octet-stream" \
  --data-binary @"$LOCAL_FILE"
