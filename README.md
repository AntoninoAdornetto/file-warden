# File Warden

# Overview

File Warden is a monitoring tool designed to track and respond to file system events within specified directories. The reason I created this program was for recreational purposes. I wanted to learn more about daemons and create one myself. File warden is a managed service (`systemd`). The key features are:

- `Real time monitoring`: Observe designated paths for various file system activities, such as opening, closing, creating, deleting, reading, and writing files. 

- `Notifications`: The program utilizes `libnotify` to deliver desktop alerts whenever a monitoring path experiences a files system event.

- `Logging`: Records all file system events to `journal` alongside desktop notifications

These features allow you to monitor critical files or directories.

