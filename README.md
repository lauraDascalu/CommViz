# HTTP File Downloader – TCP/IP Client in C++

## 1. Overview

This program implements a TCP/IP client that interacts with a web server using the HTTP protocol. It can download files over plain HTTP by establishing a socket connection to a specified server, requesting a resource, and storing the response content locally.

---

## 2. Functionality

### 🔹 URL Parsing

The program begins by accepting a user-provided URL, which is parsed into:

- **Host**: The domain name or IP address of the server  
- **Resource Path (site)**: The path on the server where the file resides  
- **Filename**: Extracted from the resource path and used as the local file name to save the downloaded content

### 🔹 Processing the Server Response and Saving the Data to a File

The server responds with an HTTP message consisting of **headers** and a **body**.  
The client program reads the response, searching for the header-body separator (`\r\n\r\n`), which indicates the end of the headers.  
Only the data following this separator is written to the output file, using the previously extracted filename.

---

## 3. Testing

The program was tested in the **university campus network** environment, where **proxy servers are not required**, ensuring compatibility with the restricted network setup.

During testing, the console output displayed the parsed host, filename, and resource path extracted from the URL, followed by messages indicating the connection progress to the server. It showed the HTTP GET request sent and printed the full HTTP response received.  
The file content was saved to a local file using the correct filename.

---

##  Limitations

- The program only supports **HTTP**, not **HTTPS**
- It does **not support proxies**
