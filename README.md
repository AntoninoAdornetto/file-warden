<a id="readme-top"></a>

[![Contributors][contributors-shield]][contributors-url]
[![Forks][forks-shield]][forks-url]
[![Stargazers][stars-shield]][stars-url]
[![Issues][issues-shield]][issues-url]
[![project_license][license-shield]][license-url]



<!-- PROJECT LOGO -->
<br />
<div align="center">
  <a href="https://brainmade.org">
    <img src="https://brainmade.org/black-logo.svg" alt="Logo" width="80" height="80">
  </a>

<h3 align="center">File Warden</h3>

  <p align="center">
    Systemd-managed daemon (service) that monitors specified directories for file system events and sends desktop notifications for changes
  </p>
</div>



<!-- TABLE OF CONTENTS -->
<details>
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#about-the-project">About The Project</a>
    </li>
    <li>
      <a href="#getting-started">Getting Started</a>
      <ul>
        <li><a href="#prerequisites">Prerequisites</a></li>
        <li><a href="#manual-installation">Manual Installation</a></li>
      </ul>
    </li>
    <li><a href="#usage">Usage</a></li>
    <li><a href="#contributing">Contributing</a></li>
    <li><a href="#license">License</a></li>
  </ol>
</details>



<!-- ABOUT THE PROJECT -->
## About The Project

File Warden is a recreational programming project that has allowed me to deepen my understanding of daemons and systemd-managed services. While I have been familiar with daemons for some time and consume them regularly, I wanted to enhance my knowledge and create a small utility that I could use personally. 

The core features of File Warden are:

- **`Real time monitoring`**: Observe designated paths for various file system activities, such as opening, closing, creating, deleting, reading, and writing files. 

- **`Notifications`**: File warden utilizes [libnotify](https://gnome.pages.gitlab.gnome.org/libnotify/) to deliver desktop alerts whenever a monitoring path experiences a files system event.

- **`Logging`**: Records all file system events to [Journal](https://www.man7.org/linux/man-pages/man8/systemd-journald.service.8.html) alongside the desktop notifications


<p align="right">(<a href="#readme-top">back to top</a>)</p>

<!-- GETTING STARTED -->
## Getting Started

### Prerequisites

**NOTE**:
File Warden is **not designed to support all operating systems**. It is specifically intended for use on **Linux**, as it relies on `libnotify` to send desktop notifications to a notification daemon.

Ensure you have the following packages installed:

- `gcc` - GNU Compiler - C and C++ frontends
- `libnotify` - Library for sending desktop notifications
- `make` - GNU make utility

### Manual Installation

1. clone the repo, change directory, and build file-warden
```bash
# First, clone the repo
git clone https://github.com/AntoninoAdornetto/file-warden.git

# Next, change directory into file-warden
cd file-warden

# Last, compile the program
make
```

2. Determine where you would like to place your systemd user unit config file 
```bash
# Read the section "User Unit Search Path" in the man pages
# and select a path that you would like to copy the unit configuration to
man 5 systemd.unit
```

3. Copy the example unit file to your choosen `user search path`
```bash
# I personally use ~/.config/systemd/user but use the path you prefer
cp docs/file-warden.example.service ~/.config/systemd/user/file-warden.service
```

4. Update the ExecStart path to where your binary is located
```bash
# @TODO: Provide instructions on how to define the ExecStart path
```

4. Reload systemd user services
```bash
systemctl --user daemon-reload
```

5. Define watch paths and events via configuration file
```bash
# @TODO: Provide instructions on how to create the program configuration file
```

6. Start/enable the service
```bash
# Start the service
systemctl --user start file-warden

# or, if you prefer to have it start on boot
systemctl --user enable file-warden
```

7. Enjoy :smile:

<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- USAGE EXAMPLES -->
## Usage

TODO! 
<!--Use this space to show useful examples of how a project can be used. Additional screenshots, code examples and demos work well in this space. You may also link to more resources.-->
<!---->
<!--_For more examples, please refer to the [Documentation](https://example.com)_-->

<p align="right">(<a href="#readme-top">back to top</a>)</p>


<!-- CONTRIBUTING -->
## Contributing

TODO!
<!--Contributions are what make the open source community such an amazing place to learn, inspire, and create. Any contributions you make are **greatly appreciated**.-->
<!---->
<!--If you have a suggestion that would make this better, please fork the repo and create a pull request. You can also simply open an issue with the tag "enhancement".-->
<!--Don't forget to give the project a star! Thanks again!-->
<!---->
<!--1. Fork the Project-->
<!--2. Create your Feature Branch (`git checkout -b feature/AmazingFeature`)-->
<!--3. Commit your Changes (`git commit -m 'Add some AmazingFeature'`)-->
<!--4. Push to the Branch (`git push origin feature/AmazingFeature`)-->
<!--5. Open a Pull Request-->

<p align="right">(<a href="#readme-top">back to top</a>)</p>

### Top contributors:

<a href="https://github.com/AntoninoAdornetto/file-warden/graphs/contributors">
  <img src="https://contrib.rocks/image?repo=AntoninoAdornetto/file-warden" alt="contrib.rocks image" />
</a>



<!-- LICENSE -->
## License

TODO!
<!--Distributed under the project_license. See `LICENSE.txt` for more information.-->

<p align="right">(<a href="#readme-top">back to top</a>)</p>


<!-- MARKDOWN LINKS & IMAGES -->
<!-- https://www.markdownguide.org/basic-syntax/#reference-style-links -->
[contributors-shield]: https://img.shields.io/github/contributors/AntoninoAdornetto/file-warden.svg?style=for-the-badge
[contributors-url]: https://github.com/AntoninoAdornetto/file-warden/graphs/contributors
[forks-shield]: https://img.shields.io/github/forks/AntoninoAdornetto/file-warden.svg?style=for-the-badge
[forks-url]: https://github.com/AntoninoAdornetto/file-warden/network/members
[stars-shield]: https://img.shields.io/github/stars/AntoninoAdornetto/file-warden.svg?style=for-the-badge
[stars-url]: https://github.com/AntoninoAdornetto/file-warden/stargazers
[issues-shield]: https://img.shields.io/github/issues/AntoninoAdornetto/file-warden.svg?style=for-the-badge
[issues-url]: https://github.com/AntoninoAdornetto/file-warden/issues
[license-shield]: https://img.shields.io/github/license/AntoninoAdornetto/file-warden.svg?style=for-the-badge
[license-url]: https://github.com/AntoninoAdornetto/file-warden/blob/master/LICENSE.txt
[product-screenshot]: images/screenshot.png

# File Warden

# Overview

File Warden is a file monitoring tool designed to track and respond to file system events within specified directories. I created the program for recreational purposes as my primary goal
was to learn more about Daemons and systemd managed services. I also found that having a tool 

File Warden is a monitoring tool designed to track and respond to file system events within specified directories. The reason I created this program was for recreational purposes. I wanted to learn more about daemons and create one myself. File warden is a managed service (`systemd`). The key features are:

- `Real time monitoring`: Observe designated paths for various file system activities, such as opening, closing, creating, deleting, reading, and writing files. 

- `Notifications`: The program utilizes `libnotify` to deliver desktop alerts whenever a monitoring path experiences a files system event.

- `Logging`: Records all file system events to `journal` alongside desktop notifications

These features allow you to monitor critical files or directories.

