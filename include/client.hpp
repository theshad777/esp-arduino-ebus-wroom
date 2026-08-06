#pragma once

#include <cstdint>

uint32_t getDbgTcpRoAccepted();
uint32_t getDbgTcpRoPushAttempts();
uint32_t getDbgTcpRoBytesSent();

bool handleNewClient(int serverFd, int clients[]);

bool startClientRuntime();
void stopClientRuntime();

void handleClient(int* clientFd);
int pushClient(int* clientFd, uint8_t byte);

void handleClientEnhanced(int* clientFd);
int pushClientEnhanced(int* clientFd, uint8_t c, uint8_t d, bool log);

