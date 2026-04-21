#!/bin/bash

# Find the actual plugin directory
PLUGIN_DIR=$(mysql --help | grep 'plugin-dir' | awk '{print $2}')

# Create symbolic link if paths are different
if [ "$PLUGIN_DIR" != "/usr/lib/mysql/plugin" ]; then
    mkdir -p /usr/lib/mysql/plugin
    ln -sf /usr/lib/mysql/plugin/* $PLUGIN_DIR/
fi

# Execute the default MySQL entrypoint
exec docker-entrypoint.sh "$@" 