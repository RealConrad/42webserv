<div align="center">
  <h1>
    📗 Webserv
  </h1>
  <p>
    <b><i>A versatile and efficient C++ web server featuring non-blocking sockets, CGI support, and HTTP functionalities.</i></b>
  </p>
  <p>
    Developed using&nbsp&nbsp
    <a href="https://skillicons.dev">
      <img src="https://skillicons.dev/icons?i=cpp,vscode" />
    </a>
  </p>
</div>

<br />

## Table Of Contents
- [Features](#features)
- [Installation](#installation)
- [Contributors](#contributors)
- [Resources](#resources)
- [License](#license)

## Features
- **Non-Blocking Sockets:** Utilizes non-blocking socket programming to handle multiple simultaneous connections without the need for multi-threading.
- **HTTP Methods:** Supports GET, POST, and DELETE requests, allowing it to serve, accept, and delete data respectively.
- **CGI Support:** CGI support to execute programs and scripts (`.py` only), enabling dynamic content generation.
- **Directory Listing:** Automatically lists the contents of a directory when no index file is found.
- **Error Pages:** Appropriate error responses/status codes (like 200, 404 or 500). For valid/invalid requests.
- **Redirection:** Supports HTTP redirections.
- **Logger:** Includes a comprehensive logging system to track requests, responses, and server information.
- **Virtual Hosts:** Virtual hosting allows multiple domains to be hosted from a single server config.
- **UI for Testing:** A basic yet visually appealing UI is provided to test and demonstrate server functionality.
- **Documentation:** All functions have doxygen comments to ensure readability and to better understand the underlying logic at a glance.

## Installation
Ensure you have a C++ compiler installed beforehand

1. Clone the repo
```bash
git clone https://github.com/RealConrad/42webserv.git
```

2. Enter directory and compile program
```bash
cd 42webserv
make
```
3. Running the server
> NOTE: If no config is specified, it will run the default config located at: `/configs/default.config`. You can specify a config file with `./webserv <path-to-config-file>`
```bash
./webserv
```

4. Open it on chrome
> NOTE: Change host name and port to whats specified inside the config file. The example shown will work for `default.config`
```
http://localhost:8080
```

## Contributors
- [RealConrad](https://github.com/RealConrad)
- [kglebows](https://github.com/kglebows)

## Resources
For a list of resources used, check out our wiki page [here](https://github.com/RealConrad/42webserv/wiki).

## License
Click [here](https://choosealicense.com/licenses/mit/) or view the License Tab.
