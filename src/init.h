// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The sweetcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef sweetcoin_INIT_H
#define sweetcoin_INIT_H

#include "wallet.h"

extern CWallet* pwalletMain;

void StartShutdown();
void Shutdown(void* parg);
bool AppInit(int argc, char* argv[]);
bool AppInit2();
std::string HelpMessage();

#endif
