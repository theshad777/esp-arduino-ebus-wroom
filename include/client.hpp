#pragma once

#include <cstdint>

uint32_t getDbgTcpRoAccepted();
uint32_t getDbgTcpRoPushAttempts();
uint32_t getDbgTcpRoBytesSent();
uint32_t getDbgTcpAcceptLoops();
uint32_t getDbgSocketPeekData();
uint32_t getDbgSocketPeekAgain();
uint32_t getDbgSocketPeekClosed();
uint32_t getDbgSocketPeekOtherErr();
int32_t getDbgSocketPeekLastErrno();
uint32_t getDbgSocketSendOk();
uint32_t getDbgSocketSendErr();
int32_t getDbgSocketSendLastErrno();
uint32_t getDbgEnhancedSecondByteMissing();
uint32_t getDbgEnhancedSignatureError();
uint32_t getDbgEnhancedArbRequestSet();
uint32_t getDbgEnhancedCmdInit();
uint32_t getDbgEnhancedCmdSend();
uint32_t getDbgEnhancedCmdStart();
uint32_t getDbgEnhancedCmdStartSyn();
uint32_t getDbgEnhancedCmdStartAddress();
uint32_t getDbgEnhancedCmdInfo();
uint32_t getDbgEnhancedFirstSignatureError();
uint32_t getDbgEnhancedLastStartAddress();

bool handleNewClient(int serverFd, int clients[]);

bool startClientRuntime();
void stopClientRuntime();

void handleClient(int* clientFd);
int pushClient(int* clientFd, uint8_t byte);

void handleClientEnhanced(int* clientFd);
int pushClientEnhanced(int* clientFd, uint8_t c, uint8_t d, bool log);
