#ifndef LOG_H
#define LOG_H

#include <stdio.h>

// ANSI color codes
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define RESET "\033[0m"

// Logging macros
#define INFO(fmt, ...) printf(BLUE "[+] " RESET fmt "\n", ##__VA_ARGS__)
#define SUCCESS(fmt, ...) printf(GREEN "[✓] " RESET fmt "\n", ##__VA_ARGS__)
#define ERROR(fmt, ...) printf(RED "[!] " RESET fmt "\n", ##__VA_ARGS__)
#define WARN(fmt, ...) printf(YELLOW "[*] " RESET fmt "\n", ##__VA_ARGS__)

#endif