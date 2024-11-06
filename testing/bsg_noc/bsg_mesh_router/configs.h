#pragma once 

#ifndef TOTAL_CYCLES
#define TOTAL_CYCLES 100
#endif

#ifndef EDGE_LP
#define EDGE_LP 2
#endif

#ifndef DATA_LP
#define DATA_LP 4
#endif

#define EDGE_SZ (1 << EDGE_LP)

/********************************************
 * Configs for SndFreqFn
 *******************************************/
#ifndef SndFreqFn_CFG
#define SndFreqFn_CFG 0
#endif

#ifndef SndFreqFn_P
#define SndFreqFn_P 0.5
#endif

#if SndFreqFn_CFG == 0
#define SndFreqFn_initializer AlwaysSndFn::initializer()
#elif SndFreqFn_CFG == 1
#define SndFreqFn_initializer RandomSndFn::initializer(SndFreqFn_P)
#endif

/********************************************
 * Configs for RcvFreqFn
 *******************************************/
#ifndef RcvFreqFn_CFG
#define RcvFreqFn_CFG 0
#endif

#ifndef RcvFreqFn_P
#define RcvFreqFn_P 0.5
#endif

#if RcvFreqFn_CFG == 0
#define RcvFreqFn_initializer AlwaysRcvFn::initializer()
#elif RcvFreqFn_CFG == 1
#define RcvFreqFn_initializer RandomRcvFn::initializer(RcvFreqFn_P)
#endif

/********************************************
 * Configs for SndDestFn
 *******************************************/
#ifndef SndDestFn_CFG
#define SndDestFn_CFG 2
#endif

#ifndef SndDestFn_K
#define SndDestFn_K 4
#endif

#ifndef SndDestFn_BLK
#define SndDestFn_BLK 3
#endif

#if SndDestFn_CFG == 0
#define SndDestFn_initializer AllToOneSndFn::initializer(rand() % EDGE_SZ, rand() % EDGE_SZ)
#elif SndDestFn_CFG == 1
#define SndDestFn_initializer RandomKSndFn<EDGE_SZ>::initializer(SndDestFn_K)
#elif SndDestFn_CFG == 2
#define SndDestFn_initializer RightOrBottomSndFn<EDGE_SZ>::initializer()
#elif SndDestFn_CFG == 3
#define SndDestFn_initializer BlockSndFn<SndDestFn_BLK>::initializer()
#endif