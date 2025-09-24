# ft_ping

A custom implementation of the ping utility program, developed as part of the 42 school curriculum.

## Description

ft_ping tests network connectivity by sending ICMP echo request packets to a target host and measuring round-trip times. Based on the inetutils-2.0 ping implementation, it provides detailed network diagnostics and packet analysis capabilities.

## Features

- IPv4 address and hostname support
- DNS resolution with FQDN display
- Round-trip time statistics (min/avg/max/stddev)
- Verbose mode with detailed packet information
- Customizable TTL, timeout, and deadline options
- Comprehensive ICMP error message handling
- Signal handling for graceful termination

## Requirements

- Root privileges (uses raw sockets)
- Linux environment (Debian >= 7.0)
- GCC compiler with math library

## Usage

```bash
sudo ./ft_ping [options] <destination>
sudo ./ft_ping -h
```

### Options

- `<destination>` - DNS name or IP address
- `-h` - Print help and exit
- `-v` - Verbose output (shows packet details and errors)
- `--ttl=<value>` - Set Time To Live (1-255)
- `-w <deadline>` - Stop after deadline seconds
- `-W <timeout>` - Time to wait for response

### Examples

```bash
sudo ./ft_ping google.com
sudo ./ft_ping -v 8.8.8.8
sudo ./ft_ping --ttl=30 localhost
sudo ./ft_ping -w 10 -W 2 example.com
```

## Compilation

```bash
make
```

## Implementation Details

- Uses raw ICMP sockets for packet transmission
- Implements RFC 1071 checksum calculation
- Real-time RTT measurement with nanosecond precision
- Comprehensive ICMP type/code error handling
- Packet loss and error statistics
- Memory management with RTT linked list

## 42 Project

This project is part of the 42 school UNIX network programming curriculum, focusing on:
- Network protocols (ICMP, IP)
- Raw socket programming
- System calls and signal handling
- Statistical analysis and time measurement
- Error handling and packet analysis
